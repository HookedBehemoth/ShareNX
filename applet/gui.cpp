#include "gui.hpp"

#include "elements/elm_filteritem.hpp"
#include "elements/elm_lazyimage.hpp"
#include "elements/grid_layout.hpp"
#include "imgur.hpp"
#include "translation/translation.hpp"

#include <album.hpp>
#include <borealis.hpp>

#include "logo_bin.h"

namespace gui {

    namespace {

        bool found_language      = false;
        bool init_album_accessor = false;

    }

    bool Initialize() {
        /* Set log level */
        brls::Logger::setLogLevel(brls::LogLevel::DEBUG);

        if (!brls::Application::init())
            return false;

        /* Official software actually sets a function pointer that's called in nn::applet::StartLibraryApplet instead of setting the jump flag */
        libappletSetJumpFlag(appletGetAppletType() == AppletType_LibraryApplet);

        socketInitializeDefault();
        setInitialize();
        nsInitialize();
        init_album_accessor = R_SUCCEEDED(capsaInitialize());

        found_language = Translation::DetectSystemLanguage();
        if (!found_language)
            Translation::SetLanguage(Translation::Language::English_US);

        album::Initialize();

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

        auto QuitAction = [](brls::View *) { brls::Application::quit(); };

        if (!found_language) {
            brls::Dialog *dialog = new brls::Dialog("Failed to get system language!\nDefaulted to en-US!");
            dialog->addButton(~CONTINUE, [dialog](brls::View *view) {
                dialog->close();
            });
            dialog->addButton(~EXIT, QuitAction);
            dialog->open();
        }

        if (!init_album_accessor) {
            brls::Dialog *dialog = new brls::Dialog(~ACCESSOR_INIT);
            dialog->addButton(~EXIT, QuitAction);
            dialog->open();
        }
    }

    void Start() {
        // Run the app
        while (brls::Application::mainLoop())
            ;
    }

    void Cleanup() {
        album::Exit();
        capsaExit();
        nsExit();
        setExit();
        socketExit();
    }

}
