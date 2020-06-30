#include "translation.hpp"

#include <cstdio>
#include <switch.h>

namespace album {

    #include "languages/chinese.inc"
    #include "languages/dutch.inc"
    #include "languages/english.inc"
    #include "languages/french.inc"
    #include "languages/german.inc"
    #include "languages/italian.inc"
    #include "languages/japanese.inc"
    #include "languages/korean.inc"
    #include "languages/polish.inc"
    #include "languages/russian.inc"
    #include "languages/spanish.inc"

    namespace lang {

        constexpr static const char *const *translations[Total_Languages] = {
            [Japanese]      = japanese,
            [English_US]    = english,
            [French]        = french,
            [German]        = german,
            [Italian]       = italian,
            [Spanish]       = spanish,
            [Chinese_CN]    = chinese,
            [Korean]        = korean,
            [Dutch]         = dutch,
            [Polish]        = polish,
            [Russian]       = russian,
            [Chinese_TW]    = chinese,
            [English_GB]    = english,
            [French_CA]     = french,
            [Spanish_Latin] = spanish,
            [Chinese_HANS]  = chinese,
            [Chinese_HANT]  = chinese,
        };

        static const char *const *current = translations[English_US];

        const char *Translate(String key) {
            /* Return a snowflake if the key is out of range. */
            if (key >= String::Total_Strings)
                return "\ue152";

            /* Return a snowman if the string isn't defined. */
            const char *temp = current[key];
            if (!temp)
#ifdef __DEBUG__
                return "\u2603";
#else
                return english[key];
#endif

            /* Return translated string. */
            return temp;
        }

        bool SetLanguage(Language lang) {
            /* Skip invalid language. */
            if (lang >= Language::Total_Languages)
                return false;

            /* Overwrite current translation array. */
            current = translations[lang];
            return true;
        }

        void DetectSystemLanguage() {
            bool set = false;
            ::SetLanguage ourLang;
            u64 language_code;
            if (R_SUCCEEDED(setGetSystemLanguage(&language_code)))
                if (R_SUCCEEDED(setMakeLanguage(language_code, &ourLang)))
                    set = SetLanguage(Language(ourLang));
            if (!set || current[0] == nullptr)
                SetLanguage(Language::English_US);
        }

        void DebugPrintAllStrings() {
            for (u32 i = 0; i < Total_Languages; i++) {
                SetLanguage(Language(i));
                for (u32 j = 0; j < Total_Strings; j++) {
                    std::printf("%s\n", Translate(String(j)));
                }
            }
        }

    }

}
