#pragma once

#include <string>
#include <vector>

extern "C" {
    #include <switch/services/capsa.h>
}

namespace caps {

std::string dateToString(const CapsAlbumFileDateTime& date);
std::string entryToFileName(const CapsAlbumEntry& entry);
Result getThumbnail(u64* width, u64* height, const CapsAlbumEntry& entry, void* image, u64 image_size);
Result getImage(u64* width, u64* height, const CapsAlbumEntry& entry, void* image, u64 image_size);
Result getFile(const CapsAlbumEntry& entry, void* filebuf);
std::pair<Result,std::vector<CapsAlbumEntry>> getEntries(const CapsAlbumStorage& storage);
std::vector<CapsAlbumEntry> getAllEntries();
Result moveFile(const CapsAlbumEntry& entry);

class MovieReader {
public:
    MovieReader(const CapsAlbumEntry& entry);
    ~MovieReader();
    u64 GetStreamSize();
    size_t Read(char* buffer, size_t max);
private:
    u64 streamSize = 0;
    CapsAlbumEntry m_entry;
    size_t progress = 0;
    u64 stream = 0;
    void* readBuffer;
    u64 bufferSize = 0x40000;
    Result error = 0;
};

}