#include "util/caps.hpp"
#include <fstream>
#include <algorithm>

namespace caps {

std::string dateToString(const CapsAlbumFileDateTime& date) {
    const char* temp ="%04d.%02d.%02d %02d:%02d:%02d";
    char dateString[20];
    snprintf(dateString, 20, temp, date.year, date.month, date.day, date.hour, date.minute, date.second);
    return std::string(dateString);
}

Result getThumbnail(u64* width, u64* height, const CapsAlbumEntry& entry, void* raw_buffer, u64 raw_buffer_size) {
    void* work_buffer = malloc(entry.size);
    Result rc = capsaLoadAlbumScreenShotThumbnailImage(width, height, entry.file_id, work_buffer, entry.size, raw_buffer, raw_buffer_size);
    printf("capsaLoadAlbumScreenShotThumbnailImage: 0x%x\n", rc);
    free(work_buffer);
    return rc;
}

Result getImage(u64* width, u64* height, const CapsAlbumEntry& entry, void* raw_buffer, u64 raw_buffer_size) {
    void* work_buffer = malloc(entry.size);
    Result rc = capsaLoadAlbumScreenShotImage(width, height, entry.file_id, work_buffer, entry.size, raw_buffer, raw_buffer_size);
    printf("capsaLoadAlbumScreenShotImage: 0x%x\n", rc);
    free(work_buffer);
    return rc;
}

Result getFile(const CapsAlbumEntry& entry, void* buffer) {
    u64 tmp;
    return capsaLoadAlbumFile(entry.file_id, &tmp, buffer, entry.size);
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
    Result rc = capsaStorageCopyAlbumFile(entry.file_id, dest);
    printf("copy: 0x%x\n", rc);
    if (R_SUCCEEDED(rc)) {
        rc = capsaDeleteAlbumFile(entry.file_id);
        printf("delete: 0x%x\n", rc);
    }
    return rc;
}

MovieReader::MovieReader(const CapsAlbumEntry& entry) : m_entry(entry) {
}

MovieReader::~MovieReader() {
}

size_t MovieReader::Read(char* buffer, size_t max) {
    size_t remaining = m_entry.size - this->progress;
    if (max > remaining)
        max = remaining;
    /* STUFF */
    this->progress -= max;
    return 0;
}

}