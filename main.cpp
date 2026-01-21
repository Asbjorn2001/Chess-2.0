#include <unistd.h>
#include <cmath>
#include <cstdlib>
#include "src/gui.h"

int main(int argc, char* argv[]) {
    Bitboards::init();

    ChessGUI().loop();

    return EXIT_SUCCESS;
}
