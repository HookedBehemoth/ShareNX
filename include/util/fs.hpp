#pragma once
#include <switch.h>
#include <vector>
#include <string>

namespace fs {

class SD {
public:
    SD();
    ~SD();
    std::vector<FsDirectoryEntry> GetHoster();
private:
    FsFileSystem sd_fs;
};

}