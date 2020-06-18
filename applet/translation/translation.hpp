#pragma once

namespace album {

    enum String {
        /* Misc */
        OK,
        BACK,
        CANCEL,
        FINISHED,
        COPY,
        DELETE,
        DELETE_ITEMS,
        EXIT,
        ERROR,
        POST,
        HIDE,
        PLAY,
        PAUSE,
        CONTINUE,
        FATAL_EXCEPTION,
        REASON,
        /* Results */
        ACCESSOR_INIT,
        TOKEN_MISSING,
        WEB_APPLET_MODE,
        WEB_EXIT_REASON,
        CONFIG_SAVED_FMT,
        /* */
        DELETE_INFO,
        NAND,
        SD,
        COPY_SD_TO_NAND,
        COPY_NAND_TO_SD,
        FILTER,
        FILTER_ALL,
        FILTER_SCREENSHOTS,
        FILTER_VIDEOS,
        FILTER_NAND,
        FILTER_SD,
        FILTER_OTHER,
        SECONDS_SHORT,
        UPLOADER,
        /* String count. */
        Total_Strings,
    };

    namespace lang {

        enum Language {
            /* Languages offically supported by N. */
            Japanese,
            English_US,
            French,
            German,
            Italian,
            Spanish,
            Chinese_CN,
            Korean,
            Dutch,
            Polish,
            Russian,
            Chinese_TW,
            English_GB,
            French_CA,
            Spanish_Latin,
            Chinese_HANS,
            Chinese_HANT,
            /* Additional languages not officially supported by N. */
            Vong,
            /* Total languages. */
            Total_Languages,
        };

        const char *Translate(String key);
        bool SetLanguage(Language lang);
        void DetectSystemLanguage();
        void DebugPrintAllStrings();

    }

    /* This operator is stupid but I don't want to write Translate() everywhere. */
    inline const char *operator~(String key) {
        return lang::Translate(key);
    }

}
