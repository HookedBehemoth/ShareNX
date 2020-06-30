#include "gui.hpp"

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
