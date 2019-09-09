#include "MainApplication.hpp"
#include "utils.hpp"

std::string imgPath;
using namespace std::filesystem;
int main(int argc, char* argv[])
{
    romfsInit();
    imgPath = "sdmc:/";
	imgPath += getAlbumPath();

    auto main = MainApplication::New();
    main->Show();
    romfsExit();
    return 0;
}
