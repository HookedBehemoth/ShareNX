#include "gui.hpp"

#include "exceptions.hpp"
#include "translation/translation.hpp"
#include "ui/album_adapter.hpp"
#include "ui/brls_ext/recycler_view.hpp"
#include "ui/filter_item.hpp"
#include "util/custom_config.hpp"

#include <album.hpp>
#include <borealis.hpp>
#include <fmt/core.h>

/* Momiji */
#include "logo_bin.h"

namespace album {

    namespace {

        bool init_album_accessor = false;

        typedef std::string (*cfg_callback)();

        bool MakeConfig(cfg_callback cb) {
            std::string msg;
            try {
                msg = fmt::format(~CONFIG_SAVED_FMT, cb());
                UpdateHoster();
            } catch (Result rc) {
                msg = fmt::format("{}: 2{:03}-{:04}", ~ERROR, R_MODULE(rc), R_DESCRIPTION(rc));
            } catch (String desc) {
                msg = lang::Translate(desc);
            } catch (std::exception &e) {
                msg = e.what();
            }
            brls::Dialog *dialog = new brls::Dialog(msg);
            dialog->addButton(~BACK, [dialog](brls::View *) { dialog->close(); });
            dialog->setCancelable(true);

            dialog->open();
            return true;
        }

        bool OpenHosterGui() {
            brls::AppletFrame *filterFrame = new brls::AppletFrame(220, 220);
            filterFrame->setTitle("ShareNX \uE134");
            filterFrame->setIcon(logo_bin, logo_bin_size);

            brls::List *filterList = new brls::List();

            auto imgurButton = new brls::ListItem("Imgur", "Log into an Imgur Account or create a new one.\nRequires Application override.");
            imgurButton->registerAction(~OK, brls::Key::A, [] { return MakeConfig(&GenerateImgurConfig); });
            filterList->addView(imgurButton);

            auto elixireButton = new brls::ListItem("Elixi.re", "Log into your Elixi.re account.");
            elixireButton->registerAction(~OK, brls::Key::A, [] { return MakeConfig(&GenerateElixireConfig); });
            filterList->addView(elixireButton);

            filterFrame->setContentView(filterList);
            brls::Application::pushView(filterFrame);

            return true;
        }

        bool OpenFilterGui() {
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

            /* Count entries by application ID */
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
        }

        void OpenMainGui() {
            brls::AppletFrame *albumFrame = new brls::AppletFrame(true, true);
            {
                albumFrame->setTitle("ShareNX \uE134");
                albumFrame->setIcon(logo_bin, logo_bin_size);

                auto recyclerView     = new brls::RecyclerView();
                auto thumbnailAdapter = new ThumbnailAdapter();
                auto view             = recyclerView->get();

                view.setAdapter(thumbnailAdapter);
                FilterListItem::setAdapter(thumbnailAdapter);

                view.registerAction(~FILTER, brls::Key::Y, &OpenFilterGui);
                view.registerAction("Custom Config", brls::Key::MINUS, &OpenHosterGui);

                albumFrame->setContentView(recyclerView);
            }

            // Add the root view to the stack
            brls::Application::pushView(albumFrame);
        };

    }

    bool Initialize() {
        /* Set log level */
        brls::Logger::setLogLevel(brls::LogLevel::DEBUG);

        brls::Application::setQuitOnPopRoot(true);
        brls::Application::setGlobalQuit(false);

        if (!brls::Application::init(WINDOW_NAME))
            return false;

        /* Official software actually sets a function pointer that's called in nn::applet::StartLibraryApplet instead of setting the jump flag */
        /* Libnx doesn't check on that flag. Modified locally. */
        libappletSetJumpFlag(appletGetAppletType() == AppletType_LibraryApplet);

        accountInitialize(AccountServiceType_Application);
        socketInitializeDefault();
        setInitialize();
        nsInitialize();
        init_album_accessor = R_SUCCEEDED(capsaInitialize());

        lang::DetectSystemLanguage();

        InitializeHoster();

        return true;
    }

    void MakeGui() {
        OpenMainGui();

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
        accountExit();
    }

}
