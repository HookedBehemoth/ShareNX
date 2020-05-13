#pragma once

#include <string>
#include <vector>

extern "C" {
#include <switch/services/capsa.h>
}

namespace album {

    std::string dateToString(const CapsAlbumFileDateTime &date);
    std::string entryToFileName(const CapsAlbumEntry &entry);
    Result getThumbnail(u64 *width, u64 *height, const CapsAlbumEntry &entry, void *image, u64 image_size);
    Result getImage(u64 *width, u64 *height, const CapsAlbumEntry &entry, void *image, u64 image_size);
    Result getFile(const CapsAlbumEntry &entry, void *filebuf);
    std::vector<CapsAlbumEntry> getEntries(CapsAlbumStorage storage);
    std::vector<CapsAlbumEntry> getAllEntries();

    namespace MovieReader {

        Result Start(const CapsAlbumFileId &file_id);
        Result Close();
        u64 GetStreamSize();
        size_t Read(char *buffer, size_t size, size_t nitems, void *);

    };

}