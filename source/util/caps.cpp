#include "util/caps.hpp"
#include <fstream>
#include <algorithm>
#include <string.h>

namespace caps {

std::string dateToString(const CapsAlbumFileDateTime& date) {
    const char* temp ="%04d.%02d.%02d %02d:%02d:%02d";
    char dateString[20];
    snprintf(dateString, 20, temp, date.year, date.month, date.day, date.hour, date.minute, date.second);
    return std::string(dateString);
}

std::string entryToFileName(const CapsAlbumEntry& entry) {
    auto& date = entry.file_id.datetime;
    const char* temp ="%04d.%02d.%02d %02d:%02d:%02d.%s";
    char fileName[24];
    snprintf(fileName, 24, temp, date.year, date.month, date.day, date.hour, date.minute, date.second, entry.file_id.content == CapsAlbumFileContents_ScreenShot ? "jpg": "mp4");
    return std::string(fileName);
}

Result getThumbnail(u64* width, u64* height, const CapsAlbumEntry& entry, void* image, u64 image_size) {
    void* workbuf = malloc(entry.size);
    Result rc = capsaLoadAlbumScreenShotThumbnailImage(width, height, &entry.file_id, image, image_size, workbuf, entry.size);
    printf("capsaLoadAlbumScreenShotThumbnailImage: 0x%x\n", rc);
    free(workbuf);
    return rc;
}

Result getImage(u64* width, u64* height, const CapsAlbumEntry& entry, void* image, u64 image_size) {
    void* workbuf = malloc(entry.size);
    Result rc = capsaLoadAlbumScreenShotImage(width, height, &entry.file_id, image, image_size, workbuf, entry.size);
    printf("capsaLoadAlbumScreenShotImage: 0x%x\n", rc);
    free(workbuf);
    return rc;
}

Result getFile(const CapsAlbumEntry& entry, void* filebuf) {
    u64 tmp;
    return capsaLoadAlbumFile(&entry.file_id, &tmp, filebuf, entry.size);
}

std::pair<Result,std::vector<CapsAlbumEntry>> getEntries(const CapsAlbumStorage& storage) {
    u64 count;
    std::vector<CapsAlbumEntry> out_vector;
    Result rc = capsaGetAlbumFileCount(storage, &count);
    printf("capsaGetAlbumFileCount %ld\n", count);
    if (R_SUCCEEDED(rc)) {
        CapsAlbumEntry entries[count];
        rc = capsaGetAlbumFileList(storage, &count, entries, sizeof(entries));
        printf("capsaGetAlbumFileList %ld\n", count);
        out_vector.reserve(count);
        if (R_SUCCEEDED(rc)) {
            for (u64 i = 0; i < count; i++) {
                out_vector.push_back(entries[i]);
            }
        }
    }
    return { rc, out_vector };
}

std::vector<CapsAlbumEntry> getAllEntries() {
    auto [nand_rc,nand_entries] = getEntries(CapsAlbumStorage_Nand);
    auto [sd_rc,sd_entries] = getEntries(CapsAlbumStorage_Sd);
    if (R_SUCCEEDED(nand_rc)) {
        if (R_SUCCEEDED(sd_rc)) {
            std::vector<CapsAlbumEntry> combined;
            combined.reserve(nand_entries.size() + sd_entries.size());
            combined.insert(combined.begin(), nand_entries.begin(), nand_entries.end());
            combined.insert(combined.end(),sd_entries.begin(),sd_entries.end());
            std::sort(combined.begin(), combined.end(), [](const CapsAlbumEntry& a, const CapsAlbumEntry& b) {
                auto& date_a = a.file_id.datetime;
                auto& date_b = b.file_id.datetime;
                if (date_a.year          !=  date_b.year) {
                    return date_a.year   >   date_b.year;
                } else if (date_a.month  !=  date_b.month) {
                    return date_a.month  >   date_b.month;
                } else if (date_a.day    !=  date_b.day) {
                    return date_a.day    >   date_b.day;
                } else if (date_a.hour   !=  date_b.hour) {
                    return date_a.hour   >   date_b.hour;
                } else if (date_a.minute !=  date_b.minute) {
                    return date_a.minute >   date_b.minute;
                } else if (date_a.second !=  date_b.second) {
                    return date_a.second >   date_b.second;
                }
                return date_a.id > date_b.id;
            });
            return combined;
        }
        return nand_entries;
    } else if (R_SUCCEEDED(sd_rc)) {
        return sd_entries;
    }
    return {};
}

Result moveFile(const CapsAlbumEntry& entry) {
    CapsAlbumStorage dest = entry.file_id.storage == CapsAlbumStorage_Nand ? CapsAlbumStorage_Sd : CapsAlbumStorage_Nand;
    Result rc = capsaStorageCopyAlbumFile(&entry.file_id, dest);
    printf("copy: 0x%x\n", rc);
    if (R_SUCCEEDED(rc)) {
        rc = capsaDeleteAlbumFile(&entry.file_id);
        printf("delete: 0x%x\n", rc);
    }
    return rc;
}

MovieReader::MovieReader(const CapsAlbumEntry& entry) : m_entry(entry) {
    printf("Init MovieReader\n");
    capsaOpenAlbumMovieStream(&this->stream, &m_entry.file_id);
    readBuffer = malloc(bufferSize);
    memset(readBuffer, 0, bufferSize);
    capsaGetAlbumMovieStreamSize(stream, &this->streamSize);
    printf("stream size: 0x%lx\n", this->streamSize);
}

MovieReader::~MovieReader() {
    printf("Exit MovieReader\n");
    free (readBuffer);
    capsaCloseAlbumMovieStream(stream);
}

u64 MovieReader::GetStreamSize() {
    return this->streamSize;
}

size_t MovieReader::Read(char* buffer, size_t max) {
    printf("progress: 0x%lx/0x%lx\tbufsize 0x%lx\t", this->progress, this->streamSize, max);
    u64 remaining = this->streamSize - this->progress;
    if (remaining <= 0) {
        printf("returning 0\n");
        return 0;
    }
    int bufferCount = this->progress / this->bufferSize;
    printf("buf %d at 0x%lx\t", bufferCount, bufferCount * this->bufferSize);
    u64 actualSize = 0;
    Result rc = capsaReadMovieDataFromAlbumMovieReadStream(this->stream, bufferCount * this->bufferSize, this->readBuffer, this->bufferSize, &actualSize);
    size_t curOffset = progress % this->bufferSize;
    u64 readSize = std::min(std::min(max, this->bufferSize - curOffset), remaining);
    void* startBuffer = readBuffer + curOffset;
    printf("buffer at 0x%lx, offset: 0x%lx\t", buffer, curOffset);
    memcpy(buffer, startBuffer, readSize);
    this->progress += readSize;
    memset(readBuffer, 0, bufferSize);
    if (R_SUCCEEDED(rc)) {
        printf("returning 0x%lx\n", readSize);
        return readSize;
    } else {
        printf("Error: 0x%x\n", rc);
        this->error = rc;
        return 0;
    }
}

}