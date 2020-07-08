#include "ns.hpp"

#include "../translation/translation.hpp"

#include <memory>

namespace album::ns {

    std::string getApplicationName(u64 titleId) {
        u64 size;
        auto data = std::make_unique<NsApplicationControlData>();

        NacpLanguageEntry *languageEntry;
        if (R_SUCCEEDED(nsGetApplicationControlData(NsApplicationControlSource_Storage, titleId, data.get(), sizeof(NsApplicationControlData), &size)) &&
            R_SUCCEEDED(nacpGetLanguageEntry(&data->nacp, &languageEntry))) {
            return languageEntry->name;
        } else {
            return ~ERROR;
        }
    }

}
