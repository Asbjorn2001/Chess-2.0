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
#include <cstdint>
#include <cstdlib>
#include <format>
#include <iostream>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include "src/ChessView.h"
#include "src/position.h"
#include "src/pretty.h"
#include "src/types.h"

void cli_game_loop(Position& pos);
void gui_game_loop(const ChessModel& model, ChessController& controller, ChessView& view);
void node_generation_test(Position&, int depth);
int generate_nodes(Position& pos, int depth);

constexpr size_t window_size = 640;

int init_SDL(SDL_Window* window, SDL_Renderer* renderer) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "Something went wrong initializing SDL\n";
        return 1;
    }

    window = {SDL_CreateWindow("Chess 2.0", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               window_size, window_size, 0)};

    if (!window) {
        std::cerr << "Error creating SDL window" << SDL_GetError() << "\n";
        return 1;
    }

    renderer = {SDL_CreateRenderer(window, -1, 0)};
    if (!renderer) {
        std::cout << stderr << "Error creating SDL Renderer\n";
        return 1;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    // SDL_Window* window{};
    // SDL_Renderer* renderer{};
    // if (init_SDL(window, renderer) != 0) {
    //     return 1;
    // }

    Bitboards::init();

    // Settings settings{640, 0, 0, true};
    // ChessModel model{ChessBoard(), settings};
    // ChessController controller{model};
    // ChessView view{renderer, model};

    Position p{};
    std::cout << p << "\n";
    node_generation_test(p, 6);

    // gui_game_loop(model, controller, view);
    // cli_game_loop(p);

    // SDL_DestroyRenderer(renderer);
    // SDL_DestroyWindow(window);

    return 0;
}

void gui_game_loop(const ChessModel& model, ChessController& controller, ChessView& view) {
    bool app_running{true};
    while (app_running) {
        if (model.is_checkmate) {
        }
        SDL_Event event{};
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: app_running = false;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        app_running = false;
                    }
                default: break;
            }
            controller.handle_event(event);
        }

        view.draw();
    }
}

void node_generation_test(Position& pos, int depth) {
    for (auto n{1}; n <= depth; ++n) {
        auto start = std::chrono::steady_clock::now();
        long nodes = generate_nodes(pos, n);
        auto end = std::chrono::steady_clock::now();
        auto duration = end - start;
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        long nodesPerSec = millis.count() ? floor(nodes * 1000 / millis.count()) : 0;
        std::cout << std::format(
            "Depth: {} ply  Result: {} Nodes  Duration: {} milliseconds {}\n", n, nodes,
            millis.count(),
            nodesPerSec ? std::format("({} nodes/sec)", nodesPerSec) : "(undefined)");
    }
}

int generate_nodes(Position& pos, int depth) {
    if (depth == 0) {
        return 1;
    }

    int num_positions{0};
    for (const auto& m : MoveList<LEGAL>(pos)) {
        pos.make_move(m);
        num_positions += generate_nodes(pos, depth - 1);
        pos.unmake_move(m);
    }

    return num_positions;
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
