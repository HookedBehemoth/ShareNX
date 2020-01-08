#pragma once

#include <string>
#include <vector>

extern "C" {
    #include <switch/services/capsa.h>
}

namespace caps {

std::string dateToString(const CapsAlbumFileDateTime& date);
Result getThumbnail(u64* width, u64* height, const CapsAlbumEntry& entry, void* buffer, u64 raw_buffer_size);
Result getImage(u64* width, u64* height, const CapsAlbumEntry& entry, void* buffer, u64 raw_buffer_size);
Result getFile(const CapsAlbumEntry& entry, void* buffer);
std::pair<Result,std::vector<CapsAlbumEntry>> getEntries(const CapsAlbumStorage& storage);
std::vector<CapsAlbumEntry> getAllEntries();
Result moveFile(const CapsAlbumEntry& entry);

class MovieReader {
public:
    MovieReader(const CapsAlbumEntry& entry);
    ~MovieReader();
    size_t Read(char* buffer, size_t max);
private:
    CapsAlbumEntry m_entry;
    size_t progress;
};

}