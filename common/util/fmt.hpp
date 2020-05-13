#pragma once

#include <cstring>
#include <string>

#include <concepts>

namespace fmt {

    /* This will have to do till P0645 comes to be. */
    template <typename... Args>
    std::string MakeString(const char *fmt, Args &&... args) {
        size_t size = std::snprintf(nullptr, 0, fmt, args...);
        char output[size + 1];
        std::memset(output, 0, sizeof(output));

        std::sprintf(output, fmt, args...);

        return output;
    }

}
