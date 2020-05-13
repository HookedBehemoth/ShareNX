#include "caps.hpp"

#include "common.hpp"

#include <memory>

bool operator<(const CapsAlbumEntry &base_a, const CapsAlbumEntry &base_b) {
    auto &a = base_a.file_id.datetime;
    auto &b = base_b.file_id.datetime;
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

bool operator>(const CapsAlbumEntry &a, const CapsAlbumEntry &b) {
    return !operator<(a, b);
}

namespace album {

    std::string dateToString(const CapsAlbumFileDateTime &date) {
        return fmt::MakeString("%04d.%02d.%02d %02d:%02d:%02d", date.year, date.month, date.day, date.hour, date.minute, date.second);
    }

    std::string entryToFileName(const CapsAlbumEntry &entry) {
        auto &date = entry.file_id.datetime;
        return fmt::MakeString("%04d.%02d.%02d %02d:%02d:%02d.%s", date.year, date.month, date.day, date.hour, date.minute, date.second, entry.file_id.content == CapsAlbumFileContents_ScreenShot ? "jpg" : "mp4");
    }

    Result getThumbnail(u64 *width, u64 *height, const CapsAlbumEntry &entry, void *image, u64 image_size) {
        auto work = std::make_unique<u8[]>(entry.size);
        return capsaLoadAlbumScreenShotThumbnailImage(width, height, &entry.file_id, image, image_size, work.get(), entry.size);
    }

    Result getImage(u64 *width, u64 *height, const CapsAlbumEntry &entry, void *image, u64 image_size) {
        auto work = std::make_unique<u8[]>(entry.size);
        return capsaLoadAlbumScreenShotImage(width, height, &entry.file_id, image, image_size, work.get(), entry.size);
    }

    Result getFile(const CapsAlbumEntry &entry, void *filebuf) {
        u64 tmp;
        std::memset(filebuf, 0, entry.size);
        return capsaLoadAlbumFile(&entry.file_id, &tmp, filebuf, entry.size);
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

    std::vector<CapsAlbumEntry> getAllEntries() {
        /* Get nand and sd entries. */
        auto nand_entries = getEntries(CapsAlbumStorage_Nand);
        auto sd_entries = getEntries(CapsAlbumStorage_Sd);

        /* Combine vector */
        std::vector<CapsAlbumEntry> combined;
        combined.reserve(nand_entries.size() + sd_entries.size());
        combined.insert(combined.begin(), nand_entries.begin(), nand_entries.end());
        combined.insert(combined.end(), sd_entries.begin(), sd_entries.end());

        /* Sort entries. */
        std::sort(combined.begin(), combined.end());
        return combined;
    }

    namespace MovieReader {

        namespace {

            constexpr const size_t AlbumMovieBufferSize = 0x40000;
            alignas(0x1000) u8 buffer[0x40000];

            u64 stream_id;
            u64 stream_size;
            u64 progress;
            int last_buffer_index;

        }

        Result Start(const CapsAlbumFileId &file_id) {
            stream_id = 0;
            R_TRY(capsaOpenAlbumMovieStream(&stream_id, &file_id));
            std::memset(buffer, 0, sizeof(buffer));

            progress = 0;
            last_buffer_index = -1;
            return capsaGetAlbumMovieStreamSize(stream_id, &stream_size);
        }

        Result Close() {
            Result rc = capsaCloseAlbumMovieStream(stream_id);
            stream_id = 0;
            stream_size = 0;
            progress = 0;
            last_buffer_index = -1;
            return rc;
        }

        u64 GetStreamSize() {
            return stream_size;
        }

        size_t Read(char *buffer, size_t size, size_t nitems, void *) {
            size_t max = size * nitems;

            /* Should we still read? */
            u64 remaining = stream_size - progress;
            if (remaining <= 0) {
                return 0;
            }

            int bufferIndex = progress / AlbumMovieBufferSize;
            u64 curOffset = progress % AlbumMovieBufferSize;
            u64 readSize = std::min({max, AlbumMovieBufferSize - curOffset, remaining});

            /* Do we need to fetch? */
            if (bufferIndex != last_buffer_index) {
                u64 actualSize = 0;

                /* Read movie data to temporary buffer. */
                if (R_FAILED(capsaReadMovieDataFromAlbumMovieReadStream(stream_id, bufferIndex * AlbumMovieBufferSize, buffer, AlbumMovieBufferSize, &actualSize))) {
                    return 0;
                }
                last_buffer_index = bufferIndex;
            }

            /* Copy movie data to output. */
            char *startBuffer = buffer + curOffset;
            std::memcpy(buffer, startBuffer, readSize);
            progress += readSize;
            return readSize;
        }

    }

}