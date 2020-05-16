#pragma once

#include <switch.h>

namespace album {

    constexpr const char *HosterConfigPath = "/config/ShareNX/";
    constexpr const char *HosterDefaultConfig = "default.txt";
    constexpr const u32 ThumbnailWidth = 320;
    constexpr const u32 ThumbnailHeight = 180;
    constexpr const u32 ThumbnailBufferSize = ThumbnailWidth * ThumbnailHeight * 4;

}