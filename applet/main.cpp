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

#include "album.hpp"

#include <cstdlib>

int main(int argc, char *argv[]) {
    /* Initialize all album functionality */
    if (!album::Initialize())
        return EXIT_FAILURE;

    /* Create GUI */
    album::MakeGui();

    /* Start GUI */
    album::Start();

    /* Cleanup */
    album::Cleanup();

    /* Exit */
    return EXIT_SUCCESS;
}
