#include "result.hpp"
#include <iomanip>

namespace result {
    std::string nintyFormat(Result rc) {
        char str[0x10];
        snprintf(str, 10, "%04d-%04d", 2000 + R_MODULE(rc), R_DESCRIPTION(rc));
        return str;
    }
    std::string hexFormat(Result rc) {
        std::stringstream stream;
        stream << "0x" << std::hex << rc;
        return stream.str();
    }
}
