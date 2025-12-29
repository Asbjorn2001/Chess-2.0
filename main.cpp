#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <format>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include "src/ChessView.h"

void cli_game_loop(ChessModel& model, ChessView& view);
void gui_game_loop(const ChessModel& model,
                   ChessController& controller,
                   ChessView& view);
void move_generation_test(int depth);
int move_generation_test(const ChessBoard& board,
                         const MoveGenerator& generator,
                         int depth);

constexpr size_t window_size = 640;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "Something went wrong initializing SDL\n";
        return 1;
    }

    auto window{SDL_CreateWindow("Chess 2.0", SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED, window_size,
                                 window_size, 0)};

    if (!window) {
        std::cerr << "Error creating SDL window" << SDL_GetError() << "\n";
        return 1;
    }

    auto renderer{SDL_CreateRenderer(window, -1, 0)};
    if (!renderer) {
        std::cout << stderr << "Error creating SDL Renderer\n";
        return 1;
    }

    try {
        Settings settings{640, 0, 0, true};
        ChessModel model{ChessBoard(), settings};
        ChessController controller{model};
        ChessView view{renderer, model};

        // move_generation_test(6);

        gui_game_loop(model, controller, view);
        // cli_game_loop(game, view);
    } catch (std::logic_error e) {
        std::cout << "Something went wrong: " << e.what() << "\n";
    }

    SDL_DestroyWindow(window);

    return 0;
}

void gui_game_loop(const ChessModel& model,
                   ChessController& controller,
                   ChessView& view) {
    bool app_running{true};
    while (app_running) {
        if (model.is_checkmate) {
        }
        SDL_Event event{};
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    app_running = false;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        app_running = false;
                    }
                default:
                    break;
            }
            controller.handle_event(event);
        }

        view.draw();
    }
}

void move_generation_test(int depth) {
    for (auto n{1}; n <= depth; ++n) {
        auto start = std::chrono::steady_clock::now();
        int res = move_generation_test(ChessBoard(), MoveGenerator(), n);
        auto end = std::chrono::steady_clock::now();
        auto duration = end - start;
        auto millis =
            std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        std::cout << std::format(
            "Depth: {} ply  Result: {} positions  Duration: {} milliseconds\n",
            n, res, millis.count());
    }
}

int move_generation_test(const ChessBoard& board,
                         const MoveGenerator& generator,
                         int depth) {
    if (depth == 0) {
        return 1;
    }

    auto position{generator.generate_position_data(board)};
    int num_positions{0};
    for (const auto& [sq, moves] : position.legal_moves) {
        for (const auto& move : moves) {
            const auto new_board = board.copy_and_move(move);
            num_positions +=
                move_generation_test(new_board, generator, depth - 1);
        }
    }

    return num_positions;
}

void cli_game_loop(ChessModel& model, ChessView& view) {
    MoveGenerator move_generator{};

    while (true) {
        std::cout << model.board << "\n\n";
        std::cout << model.board.as_fen() << "\n";

        view.draw();

        auto data = move_generator.generate_position_data(model.board);

        for (const auto& [sq, moves] : data.legal_moves) {
            std::cout << sq << ") ";
            for (const auto& move : moves) {
                std::cout << move << ", ";
            }
            std::cout << "\n";
        }

        std::cout << "Choose a move: \n";
        size_t move_num{0};
        std::cin >> move_num;
        std::cout << "your choose: " << move_num << "\n";

        // game.board.move(legal_moves[move_num - 1]);
    }
}
