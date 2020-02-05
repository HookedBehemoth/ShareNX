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
    if(R_FAILED(rc)) printf("capsaLoadAlbumScreenShotThumbnailImage: 0x%x\n", rc);
    free(workbuf);
    return rc;
}

Result getImage(u64* width, u64* height, const CapsAlbumEntry& entry, void* image, u64 image_size) {
    void* workbuf = malloc(entry.size);
    Result rc = capsaLoadAlbumScreenShotImage(width, height, &entry.file_id, image, image_size, workbuf, entry.size);
    if(R_FAILED(rc)) printf("capsaLoadAlbumScreenShotImage: 0x%x\n", rc);
    free(workbuf);
    return rc;
}

Result getFile(const CapsAlbumEntry& entry, void* filebuf) {
    u64 tmp;
    memset(filebuf, 0, entry.size);
    return capsaLoadAlbumFile(&entry.file_id, &tmp, filebuf, entry.size);
}

std::pair<Result,std::vector<CapsAlbumEntry>> getEntries(const CapsAlbumStorage& storage) {
    u64 count, loaded;
    std::vector<CapsAlbumEntry> out_vector;
    Result rc = capsaGetAlbumFileCount(storage, &count);
    printf("capsaGetAlbumFileCount %ld\n", count);
    if (R_SUCCEEDED(rc)) {
        out_vector.resize(count);
        rc = capsaGetAlbumFileList(storage, &loaded, out_vector.data(), count);
        printf("capsaGetAlbumFileList %ld\n", loaded);
        if (count != loaded) {
            printf("resizing vector to actual size\n");
            out_vector.resize(loaded);
        }
    } else {
        printf("loading entries failed with 0x%x\n", rc);
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
        printf("loading sd entries failed with 0x%x\n", sd_rc);
        return sd_entries;
    } else {
        printf("loading entries failed with 0x%x and 0x%x\n", nand_rc, sd_rc);
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
    readBuffer = (unsigned char*)malloc(bufferSize);
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
    if (this->streamSize > 0x80000000)
        return 0;
    return this->streamSize;
}

size_t MovieReader::Read(char* buffer, size_t max) {
    Result rc = 0;
    u64 remaining = this->streamSize - this->progress;
    u64 bufferIndex = this->progress / this->bufferSize;
    u64 curOffset = progress % this->bufferSize;
    u64 readSize = std::min(std::min(max, this->bufferSize - curOffset), remaining);
    double percentage = ((double)this->progress / (double)this->streamSize) * 100;
    printf("progress: 0x%lx/0x%lx %f \n", this->progress, this->streamSize, percentage);
    if (remaining <= 0) {
        return 0;
    }
    if (bufferIndex != this->lastBufferIndex) {
        u64 actualSize = 0;
        rc = capsaReadMovieDataFromAlbumMovieReadStream(this->stream, bufferIndex * this->bufferSize, this->readBuffer, this->bufferSize, &actualSize);
        this->lastBufferIndex = bufferIndex;
    }
    unsigned char* startBuffer = readBuffer + curOffset;
    memcpy(buffer, startBuffer, readSize);
    this->progress += readSize;
    if (R_SUCCEEDED(rc)) {
        return readSize;
    } else {
        printf("Error: 0x%x\n", rc);
        this->error = rc;
        return 0;
    }
}

}