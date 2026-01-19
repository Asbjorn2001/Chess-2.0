#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <unistd.h>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <format>
#include <iostream>
#include <ostream>
#include <thread>
#include "src/ChessView.h"
#include "src/gui.h"
#include "src/position.h"
#include "src/pretty.h"
#include "src/types.h"

constexpr size_t window_size = 640;

int init_SDL(SDL_Window** window, SDL_Renderer** renderer) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "Something went wrong initializing SDL\n";
        return EXIT_FAILURE;
    }

    *window = SDL_CreateWindow("Chess 2.0", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               window_size, window_size, 0);

    if (!window) {
        std::cerr << "Error creating SDL window" << SDL_GetError() << "\n";
        return EXIT_FAILURE;
    }

    *renderer = SDL_CreateRenderer(*window, -1, 0);
    if (!renderer) {
        std::cout << stderr << "Error creating SDL Renderer\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
    Bitboards::init();

    ChessGUI().loop();

    return EXIT_SUCCESS;
}
