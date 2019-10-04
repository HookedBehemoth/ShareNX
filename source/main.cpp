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

#include "ui/MainApplication.hpp"
#include "switch.h"

using namespace pu::ui::render;
int main(int argc, char* argv[])
{
    socketInitializeDefault();
#ifdef __DEBUG__
    nxlinkStdio();
#endif
    romfsInit();
    LOG("starting\n")
    try {
        auto renderer = Renderer::New(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER,
            RendererInitOptions::RendererNoSound, RendererHardwareFlags);
        auto main = scr::ui::MainApplication::New(renderer);
        main->Prepare();
        main->Show();
    } catch (std::exception& e) {
        printf("An error occurred:\n%s\n\nPress any button to exit.", e.what());
        LOG("An error occurred:\n%s", e.what());

        u64 kDown = 0;

        while (!kDown)
        {
            hidScanInput();
            kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        }
    }
    romfsExit();
    LOG("exiting\n")
    socketExit();
    return 0;
}