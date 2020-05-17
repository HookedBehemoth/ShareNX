#include "gui.hpp"

#include "imgur.hpp"
#include "translation/translation.hpp"

#include <album.hpp>
#include <borealis.hpp>
#include "elements/elm_lazyimage.hpp"

namespace gui {

    namespace {

        bool found_language = false;
        bool init_album_accessor = false;

    }

    bool Initialize() {
        /* Set log level */
        brls::Logger::setLogLevel(brls::LogLevel::DEBUG);
        brls::Application::setDisplayFramerate(true);

        if (!brls::Application::init())
            return false;

        found_language = Translation::DetectSystemLanguage();
        if (!found_language)
            Translation::SetLanguage(Translation::Language::English_US);

        init_album_accessor = R_SUCCEEDED(capsaInitialize());

        album::Initialize();

        return true;
    }

    void MakeGui() {
        brls::AppletFrame *rootFrame = new brls::AppletFrame(true, true);
        rootFrame->setTitle("ShareNX");
        rootFrame->setIcon(BOREALIS_ASSET("icon/logo.png"));

        auto *testList = new brls::BoxLayout(brls::BoxLayoutOrientation::VERTICAL);
        testList->setSpacing(10);

        for (auto &entry : album::getAllEntries())
            testList->addView(new LazyImage(entry.file_id));

        rootFrame->setContentView(testList);

        // Add the root view to the stack
        brls::Application::pushView(rootFrame);

        if (!found_language) {
            brls::Dialog *dialog = new brls::Dialog("Failed to get system language!\nDefaulted to en-US!");
            dialog->addButton("Continue", [dialog](brls::View *view) {
                dialog->close();
            });
            dialog->addButton("Exit", [](brls::View *view) {
                brls::Application::quit();
            });
            dialog->open();
        }

        if (!init_album_accessor) {
            brls::Dialog *dialog = new brls::Dialog(~ACCESSOR_INIT);
            dialog->addButton(~EXIT, [](brls::View *view) {
                brls::Application::quit();
            });
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
    }

}
