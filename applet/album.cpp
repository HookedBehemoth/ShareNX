#include "album.hpp"

#include "translation/translation.hpp"
#include "util/fmt.hpp"
#include "util/uploader.hpp"

#include <borealis.hpp>

namespace album {

    namespace {

        bool found_language = false;
        bool init_album_accessor = false;

        brls::TabFrame *rootFrame = nullptr;

        void BuildGui() {
            brls::List *testList = new brls::List();

            testList->addView(new brls::ListItem("TODO: Images", "All images should appear here in a grid layout. Ideally we'd want a recycler view."));

            brls::List *hosterList = new brls::List();

            auto *imgurCreation = new brls::ListItem("Create imgur config", "Click to make...");
            imgurCreation->getClickEvent()->subscribe([](brls::View *) {
                std::string msg;
                try {
                    msg = fmt::MakeString(~CONFIG_SAVED_FMT, upload::GenerateImgurConfig().c_str());
                } catch (Result rc) {
                    msg = fmt::MakeString("%s: 2%03d-%04d", ~ERROR, R_MODULE(rc), R_DESCRIPTION(rc));
                } catch (String desc) {
                    msg = Translation::Translate(desc);
                }
                brls::Dialog *dialog = new brls::Dialog(msg);
                dialog->addButton(~BACK, [dialog](brls::View *) { dialog->close(); });
                dialog->setCancelable(true);

                dialog->open();
            });

            hosterList->addView(imgurCreation);

            for (std::string &hoster : upload::GetHosterNameList())
                hosterList->addView(new brls::ListItem(hoster));

            rootFrame->addTab("Images", testList);
            rootFrame->addSeparator();
            rootFrame->addTab("Imgur", hosterList);
        }

    }

    bool Initialize() {
        /* Set log level */
        brls::Logger::setLogLevel(brls::LogLevel::DEBUG);
        brls::Application::setDisplayFramerate(true);

        if (!brls::Application::init())
            return false;

        found_language = false;// Translation::DetectSystemLanguage();
        init_album_accessor = R_SUCCEEDED(capsaInitialize());

        return true;
    }

    void MakeGui() {
        rootFrame = new brls::TabFrame();
        rootFrame->setTitle("ShareNX");
        rootFrame->setIcon(BOREALIS_ASSET("icon/logo.png"));

        // Add the root view to the stack
        brls::Application::pushView(rootFrame);

        if (!found_language) {
            brls::Dialog *dialog = new brls::Dialog("Failed to get system language!");
            dialog->addButton("Default to en-US", [dialog](brls::View *view) {
                found_language = Translation::SetLanguage(Translation::Language::English_US);
                dialog->close();
                BuildGui();
            });
            dialog->addButton("Exit", [](brls::View *view) {
                brls::Application::quit();
            });
            dialog->open();
            return;
        }
        if (!init_album_accessor) {
            brls::Application::pushView(new brls::Rectangle(nvgRGB(177, 0, 0)));
            brls::Dialog *dialog = new brls::Dialog(~ACCESSOR_INIT);
            dialog->addButton(~EXIT, [](brls::View *view) {
                brls::Application::quit();
            });
            dialog->open();
            return;
        }

        BuildGui();
    }

    void Start() {
        // Run the app
        while (brls::Application::mainLoop())
            ;
    }

    void Cleanup() {
        if (init_album_accessor)
            capsaExit();
    }

}
