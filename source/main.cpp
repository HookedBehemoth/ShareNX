#include "MainApplication.hpp"
#include "utils.hpp"

appcolor theme;
std::string imgPath;
int main(int argc, char* argv[])
{
    theme = *new appcolor;
    theme.WHITE = Color::FromHex("#FFFFFFFF");
    theme.RED = Color::FromHex("#6c0000FF");
    theme.DRED = Color::FromHex("#480001FF");
    theme.TOPBAR = Color::FromHex("#170909FF");
    socketInitializeDefault();
    romfsInit();
    imgPath = "sdmc:/";
	imgPath += getAlbumPath();

    auto main = MainApplication::New();
    main->Show();
    romfsExit();
    socketExit();
    return 0;
}
