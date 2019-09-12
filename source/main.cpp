/*
 * Copyright (c) 2019 screen-nx
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
