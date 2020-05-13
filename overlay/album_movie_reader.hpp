#pragma once

#include <switch.h>

namespace album {

    Result Start(const CapsAlbumFileId &file_id);
    Result Close();
    u64 GetStreamSize();
    size_t Read(char *buffer, size_t size, size_t nitems, void*);

}
