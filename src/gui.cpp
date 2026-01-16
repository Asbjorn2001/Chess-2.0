#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <cassert>
#include <optional>
#include <stdexcept>
#include "gui.h"
#include "position.h"
#include "pretty.h"
#include "types.h"

ChessGUI::ChessGUI(int size) : board({BOARD_MARGIN, BOARD_MARGIN}, size - 2 * BOARD_MARGIN) {
    assert(size >= MIN_SIZE);
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        throw std::logic_error("Something went wrong initializing SDL");
    }

    window = SDL_CreateWindow("ChessGUI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              size * 1.5, size, SDL_WINDOW_RESIZABLE);
    if (!window) {
        throw std::logic_error("Error creating SDL window");
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        throw std::logic_error("Error creating SDL Renderer\n");
    }

    Rendering::init(renderer);
    position = Position{};
}

ChessGUI::~ChessGUI() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void ChessGUI::loop() {
    while (isRunning) {
        SDL_Event event{};
        while (SDL_PollEvent(&event)) {
            handle_event(event);
        }
        render();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void ChessGUI::handle_event(SDL_Event e) {
    switch (e.type) {
        case SDL_QUIT: isRunning = false; break;
        case SDL_KEYDOWN:
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                isRunning = false;
            }
            break;
        case SDL_MOUSEBUTTONDOWN: board.handle_press(e.button); break;
        case SDL_MOUSEBUTTONUP: board.handle_release(e.button); break;
        case SDL_MOUSEMOTION: board.handle_motion(e.motion); break;

        default: break;
    }
}

void ChessGUI::update() {}

void ChessGUI::render() {
    clear(renderer, Rendering::BLACK);

    // Draw board
    board.draw(renderer);

    SDL_RenderPresent(renderer);
}

bool Board::contains(int x, int y) {
    return x > topLeft.x && x < topLeft.x + size && y > topLeft.y && y < topLeft.y + size;
}

Square Board::square_on(int x, int y) {
    assert(contains(x, y));
    Rank r = Rank(7 - (y - topLeft.y) / squareSize);
    File f = File((x - topLeft.x) / squareSize);
    return make_square(f, relative_rank(perspective, r));
}

void Board::draw(SDL_Renderer* renderer) {
    // Draw board
    auto pieces = position.board();
    if (selected.has_value() && selected->stick)
        pieces[selected->square] = NO_PIECE;
    draw_board(renderer, pieces, topLeft, size, perspective);

    // Draw optionally selected piece
    if (selected.has_value() && selected->stick) {
        int halfSS = squareSize / 2;
        draw_piece(renderer, selected->piece,
                   {selected->mouseX - halfSS, selected->mouseY - halfSS, squareSize, squareSize});
    }
}

void Board::handle_press(SDL_MouseButtonEvent e) {
    if (e.button != SDL_BUTTON_LEFT)
        return;

    if (!contains(e.x, e.y)) {
        selected = std::nullopt;
        return;
    }

    if (Square s = square_on(e.x, e.y); selected.has_value() && selected->square != s) {
        // TODO: check for legal move here.
        position.make_move({selected->square, s});
        selected = std::nullopt;
    } else if (!position.is_empty(s)) {
        // Select piece on the given square
        selected = {s, position.piece_on(s), e.x, e.y};
    }
}

void Board::handle_release(SDL_MouseButtonEvent e) {
    if (e.button != SDL_BUTTON_LEFT)
        return;

    if (!contains(e.x, e.y)) {
        selected = std::nullopt;
        return;
    }

    if (selected.has_value()) {
        if (Square s = square_on(e.x, e.y); s != selected->square) {
            position.make_move({selected->square, s});
            selected = std::nullopt;
        } else {
            selected->stick = false;
        }
    }
}

void Board::handle_motion(SDL_MouseMotionEvent e) {
    if (selected.has_value()) {
        selected->mouseX = e.x;
        selected->mouseY = e.y;
    }
}
