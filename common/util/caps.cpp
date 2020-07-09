#include "caps.hpp"

#include <album.hpp>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <memory>

bool operator>(const CapsAlbumFileId &base_a, const CapsAlbumFileId &base_b) {
    auto &a = base_a.datetime;
    auto &b = base_b.datetime;
    if (a.year != b.year) {
        return a.year > b.year;
    } else if (a.month != b.month) {
        return a.month > b.month;
    } else if (a.day != b.day) {
        return a.day > b.day;
    } else if (a.hour != b.hour) {
        return a.hour > b.hour;
    } else if (a.minute != b.minute) {
        return a.minute > b.minute;
    } else if (a.second != b.second) {
        return a.second > b.second;
    }
    return a.id > b.id;
}

inline bool operator<(const CapsAlbumFileId &a, const CapsAlbumFileId &b) {
    return !operator>(a, b);
}

inline bool operator>(const CapsAlbumEntry &base_a, const CapsAlbumEntry &base_b) {
    return operator>(base_a.file_id, base_b.file_id);
}

inline bool operator<(const CapsAlbumEntry &a, const CapsAlbumEntry &b) {
    return !operator>(a, b);
}

namespace album {

    std::string dateToString(const CapsAlbumFileDateTime &date) {
        char buffer[0x20];
        std::snprintf(buffer, sizeof(buffer), "%02d.%02d.%04d %02d:%02d:%02d",
                      date.day, date.month, date.year,
                      date.hour, date.minute, date.second);
        return buffer;
    }

    std::string MakeFileName(const CapsAlbumFileId &file_id) {
        return dateToString(file_id.datetime) + (file_id.content % 2 ? ".mp4" : ".jpg");
    }

    std::vector<CapsAlbumEntry> getEntries(CapsAlbumStorage storage) {
        u64 count, loaded;
        std::vector<CapsAlbumEntry> out_vector;

        /* Get entry count. */
        Result rc = capsaGetAlbumFileCount(storage, &count);
        if (R_SUCCEEDED(rc)) {
            out_vector.resize(count);

            /* Load entries. */
            rc = capsaGetAlbumFileList(storage, &loaded, out_vector.data(), count);
            if (R_SUCCEEDED(rc)) {
                return out_vector;
            }
        }
        return {};
    }

    const std::vector<CapsAlbumEntry> &getAllEntries(bool invalidate) {
        static std::vector<CapsAlbumEntry> combined;
        if (!invalidate && !combined.empty())
            return combined;

        combined.resize(12100);
        u64 nand_count = 0;
        u64 sd_count   = 0;

        auto *data = combined.data();
        capsaGetAlbumFileList(CapsAlbumStorage_Nand, &nand_count, data, 1100);

        data += nand_count;
        capsaGetAlbumFileList(CapsAlbumStorage_Sd, &sd_count, data, 11000);

        combined.resize(nand_count + sd_count);

        /* Sort entries. */
        std::sort(combined.begin(), combined.end(), std::greater<>());
        return combined;
    }

    Result GetLatest(CapsAlbumFileId *out, void *img, size_t size) {
        CapsAlbumFileId movie, screenshot;
        R_TRY(capsaGetLastOverlayScreenShotThumbnail(&screenshot, nullptr, img, size));

        if (hosversionBefore(4, 0, 0)) {
            *out = screenshot;
            return 0;
        }

        R_TRY(capsaGetLastOverlayMovieThumbnail(&movie, nullptr, img, size));

        /* Return latest. */
        *out = screenshot > movie ? screenshot : movie;

        return 0;
    }

    namespace MovieReader {

        namespace {

            constexpr const size_t AlbumMovieBufferSize = 0x40000;
            alignas(0x1000) u8 buffer[AlbumMovieBufferSize];

            CapsAlbumFileId file_id;
            std::atomic<int> ref_count = 0;
            u64 stream_id              = 0;
            size_t stream_size         = 0;
            size_t progress            = 0;
            s64 last_buffer_index      = -1;

        }

        Result Start(const CapsAlbumFileId &_file_id) {
            if (ref_count++) {
                if (std::memcmp(&file_id, &_file_id, sizeof(file_id)) == 0)
                    return 0;
                return MAKERESULT(211, 13);
            }

            if (stream_id != 0)
                return MAKERESULT(211, 14);

            file_id = _file_id;

            R_TRY(capsaOpenAlbumMovieStream(&stream_id, &file_id));
            std::memset(buffer, 0, AlbumMovieBufferSize);

            progress          = 0;
            last_buffer_index = -1;
            return capsaGetAlbumMovieStreamSize(stream_id, &stream_size);
        }

        Result Close() {
            Result rc = 0;
            if (--ref_count == 0) {
                puts("closing album read stream");
                rc                = capsaCloseAlbumMovieStream(stream_id);
                stream_id         = 0;
                stream_size       = 0;
                progress          = 0;
                last_buffer_index = -1;
            }
            return rc;
        }

        size_t GetStreamSize() {
            return stream_size;
        }

        size_t GetProgress() {
            return progress;
        }

        size_t Read(char *out_buffer, size_t size, size_t nitems, void *) {
            size_t max = size * nitems;

            /* Should we still read? */
            u64 remaining = stream_size - progress;
            if (remaining <= 0)
                return 0;

            s64 bufferIndex = progress / AlbumMovieBufferSize;
            u64 curOffset   = progress % AlbumMovieBufferSize;
            u64 readSize    = std::min({max, AlbumMovieBufferSize - curOffset, remaining});

            /* Do we need to fetch? */
            if (bufferIndex != last_buffer_index) {
                u64 actualSize = 0;

                /* Read movie data to temporary buffer. */
                Result rc = 0;
                if (R_FAILED(rc = capsaReadMovieDataFromAlbumMovieReadStream(stream_id, bufferIndex * AlbumMovieBufferSize, buffer, AlbumMovieBufferSize, &actualSize))) {
                    return 0;
                }
                last_buffer_index = bufferIndex;
            }

            /* Copy movie data to output. */
            const u8 *startBuffer = buffer + curOffset;
            std::memcpy(out_buffer, startBuffer, readSize);
            progress += readSize;
            return readSize;
        }

        int read_packet(void *, uint8_t *buf, int buf_size) {
            size_t size = Read((char *)buf, 1, buf_size, nullptr);
            return size == 0 ? -420 : size;
        }

        int64_t seek(void *, int64_t offset, int whence) {
            int64_t newOffset = 0;

            switch (whence) {
                case SEEK_SET:
                    newOffset = offset;
                    break;
                case SEEK_CUR:
                    newOffset = progress + offset;
                    break;
                case SEEK_END:
                    newOffset = stream_size - 1 - offset;
                    break;
                case 0x10000:
                    return stream_size;
                default:
                    return -419;
            }

            if (newOffset < 0 || static_cast<int64_t>(stream_size) < offset)
                return -420;

            last_buffer_index = -1;
            return progress   = newOffset;
        }

    }

}