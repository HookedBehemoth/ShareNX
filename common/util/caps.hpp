#pragma once

#include <string>
#include <switch.h>
#include <vector>
#include <fmt/core.h>

template <>
struct fmt::formatter<CapsAlbumFileDateTime> {
    constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const CapsAlbumFileDateTime &date, FormatContext &ctx) {
        return format_to(ctx.out(), "{:2}.{:02}.{:04} {:02}:{:02}:{:02}",
                         date.day, date.month, date.year,
                         date.hour, date.minute, date.second);
    }
};

template <>
struct fmt::formatter<CapsAlbumFileId> {
    constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const CapsAlbumFileId &fileId, FormatContext &ctx) {
        return format_to(ctx.out(), "[{:016X}, {}, {}@{}]",
                         fileId.application_id,
                         fileId.datetime,
                         fileId.content % 2 ? "Movie" : "Screenshot",
                         fileId.storage == CapsAlbumStorage_Nand ? "NAND" : "SdCard");
    }
};

namespace album {

    /* TODO: remove */
    std::string dateToString(const CapsAlbumFileDateTime &date);
    std::string MakeFileName(const CapsAlbumFileId &file_id);

    const std::vector<CapsAlbumEntry> &getAllEntries(bool invalidate = false);
    Result GetLatest(CapsAlbumFileId *out, void *img, size_t size);

    namespace MovieReader {

        Result Start(const CapsAlbumFileId &file_id);
        Result Close();
        size_t GetStreamSize();
        size_t GetProgress();
        size_t Read(char *buffer, size_t size, size_t nitems, void *);

        int read_packet(void *, uint8_t *buf, int buf_size);
        int64_t seek(void *, int64_t offset, int whence);

    };

}