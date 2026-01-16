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

void cli_game_loop(Position& pos);
void gui_game_loop(const ChessModel& model, ChessController& controller, ChessView& view);

constexpr size_t window_size = 640;

int init_SDL(SDL_Window** window, SDL_Renderer** renderer) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "Something went wrong initializing SDL\n";
        return 1;
    }

    *window = SDL_CreateWindow("Chess 2.0", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               window_size, window_size, 0);

    if (!window) {
        std::cerr << "Error creating SDL window" << SDL_GetError() << "\n";
        return 1;
    }

    *renderer = SDL_CreateRenderer(*window, -1, 0);
    if (!renderer) {
        std::cout << stderr << "Error creating SDL Renderer\n";
        return 1;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    Bitboards::init();

    ChessGUI().loop();

    return 0;
}

void cli_game_loop(Position& pos) {
    while (true) {
        std::cout << "Blockers for king: \n"
                  << pretty(pos.blockers_for_king(pos.side_to_move())) << "\n";

        std::cout << "side to move: " << (pos.side_to_move() == WHITE ? "white" : "black") << "\n";
        std::cout << pos << "\n";

        auto legalMoves = MoveList<LEGAL>(pos);
        int count = 0;
        for (auto m : legalMoves) {
            std::cout << ++count << ") " << m << "\n";
        }

        std::cout << "Choose a move: \n";
        size_t move_num{0};
        std::cin >> move_num;
        Move m = legalMoves[move_num - 1];
        std::cout << "your choose: " << m << "\n";

        pos.make_move(m);
    }
}
