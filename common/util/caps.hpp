#pragma once

#include <string>
#include <switch.h>
#include <vector>

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