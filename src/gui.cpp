#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <algorithm>
#include <cassert>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <vector>
#include "gui.h"
#include "movegen.h"
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
}

void ChessGUI::handle_event(SDL_Event e) {
    switch (e.type) {
        case SDL_QUIT: isRunning = false; break;
        case SDL_KEYDOWN:
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                isRunning = false;
            }
            break;
        case SDL_MOUSEBUTTONDOWN: handle_press(e.button); break;
        case SDL_MOUSEBUTTONUP: handle_release(e.button); break;
        case SDL_MOUSEMOTION: handle_motion(e.motion); break;

        default: break;
    }
}

void ChessGUI::update() {}

void ChessGUI::render() {
    clear(renderer, Rendering::BLACK);

    auto pieces = position.board();
    if (selected.has_value() && selected->stick)
        pieces[selected->square] = NO_PIECE;
    draw_board(renderer, pieces, board.topLeft, board.size, perspective);

    // Draw optionally selected piece
    if (selected.has_value() && selected->stick) {
        int sqSize = board.squareSize;
        int halfSqSize = sqSize / 2;
        draw_piece(renderer, selected->piece,
                   {mouseX - halfSqSize, mouseY - halfSqSize, sqSize, sqSize});
    }

    // draw_piece_selector(renderer, {300, 300}, 50, WHITE);

    SDL_RenderPresent(renderer);
}

bool Board::contains(int x, int y) {
    return x > topLeft.x && x < topLeft.x + size && y > topLeft.y && y < topLeft.y + size;
}

Square Board::square_on(int x, int y, Color perspective) {
    assert(contains(x, y));
    Rank r = Rank(7 - (y - topLeft.y) / squareSize);
    File f = File((x - topLeft.x) / squareSize);
    return make_square(f, relative_rank(perspective, r));
}

void ChessGUI::handle_press(SDL_MouseButtonEvent e) {
    if (e.button != SDL_BUTTON_LEFT)
        return;

    if (!board.contains(e.x, e.y)) {
        unselect();
        return;
    }

    if (Square s = board.square_on(e.x, e.y, perspective);
        selected.has_value() && selected->square != s) {
        try_move(selected->square, s);
        unselect();
    } else if (!position.is_empty(s)) {
        select(s);
    }
}

void ChessGUI::handle_release(SDL_MouseButtonEvent e) {
    if (e.button != SDL_BUTTON_LEFT)
        return;

    if (!board.contains(e.x, e.y)) {
        unselect();
        return;
    }

    if (selected.has_value()) {
        if (Square s = board.square_on(e.x, e.y, perspective); s != selected->square) {
            try_move(selected->square, s);
            unselect();
        } else {
            selected->stick = false;
        }
    }
}

void ChessGUI::handle_motion(SDL_MouseMotionEvent e) {
    mouseX = e.x;
    mouseY = e.y;
}

bool ChessGUI::try_move(Square from, Square to) {
    std::vector<Move> moves;
    std::ranges::copy_if(MoveList<LEGAL>(position), std::back_inserter(moves),
                         [&](const Move& m) { return m.from_sq() == from && m.to_sq() == to; });

    if (moves.size() == 1) {
        position.make_move(moves.front());
        return true;
    }

    return false;
}

void ChessGUI::select(Square s) {
    selected = {s, position.piece_on(s)};
}

void ChessGUI::unselect() {
    selected = std::nullopt;
}
