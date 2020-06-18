#include "gui.hpp"

#include "elements/elm_filteritem.hpp"
#include "elements/elm_lazyimage.hpp"
#include "elements/grid_layout.hpp"
#include "exceptions.hpp"
#include "logo_bin.h"
#include "translation/translation.hpp"

#include <album.hpp>
#include <borealis.hpp>

namespace album {

    namespace {

        bool init_album_accessor = false;

    }

    bool Initialize() {
        /* Set log level */
        brls::Logger::setLogLevel(brls::LogLevel::DEBUG);

        brls::Application::setQuitOnPopRoot(true);
        brls::Application::setGlobalQuit(false);

        if (!brls::Application::init())
            return false;

        /* Official software actually sets a function pointer that's called in nn::applet::StartLibraryApplet instead of setting the jump flag */
        /* Libnx doesn't check on that flag. Modified locally. */
        libappletSetJumpFlag(appletGetAppletType() == AppletType_LibraryApplet);

        socketInitializeDefault();
        setInitialize();
        nsInitialize();
        init_album_accessor = R_SUCCEEDED(capsaInitialize());

        lang::DetectSystemLanguage();

        InitializeHoster();

        return true;
    }

    void MakeGui() {
        brls::AppletFrame *albumFrame = new brls::AppletFrame(true, true);
        {
            albumFrame->setTitle("ShareNX");

            albumFrame->setIcon(logo_bin, logo_bin_size);

            auto *testList = new Grid();
            testList->registerAction(~FILTER, brls::Key::Y, [] {
                brls::AppletFrame *filterFrame = new brls::AppletFrame(220, 220);
                filterFrame->setTitle(~FILTER);

                brls::List *filterList = new brls::List();
                filterList->addView(new FilterListItem());
                filterList->addView(new brls::ListItemGroupSpacing());
                filterList->addView(new FilterListItem(CapsAlbumFileContents_ScreenShot));
                filterList->addView(new FilterListItem(CapsAlbumFileContents_Movie));
                filterList->addView(new FilterListItem(CapsAlbumStorage_Nand));
                filterList->addView(new FilterListItem(CapsAlbumStorage_Sd));
                filterList->addView(new brls::ListItemGroupSpacing());

                int appletCount = 0;
                std::map<u64, int> applicationMap;
                for (const auto &entry : album::getAllEntries()) {
                    if (entry.file_id.application_id < 0x010000000000ffff) {
                        appletCount++;
                    } else {
                        applicationMap[entry.file_id.application_id]++;
                    }
                }

                for (auto [tid, count] : applicationMap)
                    filterList->addView(new FilterListItem(tid, count));

                filterList->addView(new FilterListItem(0, appletCount));

                filterFrame->setContentView(filterList);
                brls::Application::pushView(filterFrame);
                return true;
            });
            testList->registerAction(~DELETE_ITEMS, brls::Key::X, [] { return true; });

            for (const auto &entry : album::getAllEntries())
                testList->addView(new LazyImage(entry.file_id));

            albumFrame->setContentView(testList);
        }

        // Add the root view to the stack
        brls::Application::pushView(albumFrame);

        if (!init_album_accessor) {
            brls::Dialog *dialog = new brls::Dialog(~ACCESSOR_INIT);
            dialog->addButton(~EXIT, [](brls::View *) { brls::Application::quit(); });
            dialog->open();
        }
    }

    void Start() {
        // Run the app
        while (brls::Application::mainLoop()) {
            auto *crashFrame = getCrashFrame();
            if (crashFrame != nullptr)
                brls::Application::pushView(crashFrame);
        }
    }

    void Cleanup() {
        ExitHoster();
        capsaExit();
        nsExit();
        setExit();
        socketExit();
    }

}
