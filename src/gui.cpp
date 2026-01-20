#include <SDL2/SDL.h>
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <iterator>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <vector>
#include "gui.h"
#include "movegen.h"
#include "position.h"
#include "types.h"

ChessGUI::ChessGUI(int size) : board({BOARD_MARGIN, BOARD_MARGIN}, size - 2 * BOARD_MARGIN) {
    assert(size >= MIN_SIZE);
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        throw std::logic_error("Something went wrong initializing SDL");
    }

    window =
        SDL_CreateWindow("ChessGUI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, size, size, 0);
    if (!window) {
        throw std::logic_error("Error creating SDL window");
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        throw std::logic_error("Error creating SDL Renderer\n");
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

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
    clear(renderer, Rendering::DARK_BROWN);

    auto pieces = position.board();
    if (selected.has_value() && selected->stick)
        pieces[selected->square] = NO_PIECE;
    draw_board(renderer, pieces, board.topLeft, board.size, perspective);

    if (selected.has_value()) {
        // Draw movable squares
        for (Square s : legal_squares_from(selected->square)) {
            draw_square(renderer, board.corner_of(s, perspective), board.sqSize,
                        Rendering::TRANSBLUE);
        }
        // Draw optionally selected piece
        if (selected->stick) {
            int sqSize = board.sqSize;
            int halfSqSize = sqSize / 2;
            draw_piece(renderer, selected->piece,
                       {mouseX - halfSqSize, mouseY - halfSqSize, sqSize, sqSize});
        }
    }

    if (promotionSelector.has_value())
        draw_piece_selector(renderer, promotionSelector->topLeft, promotionSelector->sqSize,
                            position.side_to_move(), DrawDirection::Horizontal);

    SDL_RenderPresent(renderer);
}

void ChessGUI::handle_press(SDL_MouseButtonEvent e) {
    if (e.button != SDL_BUTTON_LEFT)
        return;

    if (promotionSelector.has_value()) {
        if (promotionSelector->contains(e.x, e.y))
            position.make_move(promotionSelector->move_on(e.x, e.y));
        close_selector();
    }

    if (!board.contains(e.x, e.y)) {
        unselect();
    } else if (Square s = board.square_on(e.x, e.y, perspective);
               selected.has_value() && selected->square != s) {
        if (!try_move({selected->square, s}) && !position.is_empty(s)) {
            select(s);
        } else {
            unselect();
        }
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
            try_move({selected->square, s});
            unselect();
        } else {
            selected->stick = false;
        }
    }
}

std::vector<Square> ChessGUI::legal_squares_from(Square from) const {
    MoveList<LEGAL> moves(position);
    auto legalSquares = moves |
                        std::views::filter([&](const Move& m) { return m.from_sq() == from; }) |
                        std::views::transform([&](const Move& m) { return m.to_sq(); });

    return std::ranges::to<std::vector<Square>>(legalSquares);
}

void ChessGUI::handle_motion(SDL_MouseMotionEvent e) {
    mouseX = e.x;
    mouseY = e.y;
}

bool ChessGUI::try_move(Move move) {
    Square from = move.from_sq();
    Square to = move.to_sq();
    auto legalMoves = MoveList<LEGAL>(position);

    // Early exit on promotion selections.
    if (move.type_of() == PROMOTION && legalMoves.contains(move)) {
        position.make_move(move);
        return true;
    }

    std::vector<Move> moves;
    std::ranges::copy_if(legalMoves, std::back_inserter(moves),
                         [&](const Move& m) { return m.from_sq() == from && m.to_sq() == to; });

    // En passant, castling and normal moves.
    if (moves.size() == 1) {
        position.make_move(moves.front());
        return true;
    }

    // Move must be a promotion move, open the selector.
    if (moves.size() > 1) {
        assert(moves.size() == 4);
        open_selector(from, to);
    }

    return false;
}

void ChessGUI::select(Square s) {
    selected = {s, position.piece_on(s)};
}

void ChessGUI::unselect() {
    selected = std::nullopt;
}

void ChessGUI::open_selector(Square from, Square to) {
    int offsetX = board.sqSize * 2;
    int offsetY = board.sqSize * 3.5;
    promotionSelector = PromotionSelector({board.topLeft.x + offsetX, board.topLeft.y + offsetY},
                                          board.sqSize, from, to);
}

void ChessGUI::close_selector() {
    promotionSelector = std::nullopt;
}

bool Board::contains(int x, int y) {
    return x > topLeft.x && x < topLeft.x + size && y > topLeft.y && y < topLeft.y + size;
}

Square Board::square_on(int x, int y, Color perspective) {
    assert(contains(x, y));
    Rank r = Rank(7 - (y - topLeft.y) / sqSize);
    File f = File((x - topLeft.x) / sqSize);
    return make_square(f, relative_rank(perspective, r));
}

SDL_Point Board::corner_of(Square s, Color perspective) {
    int x = topLeft.x + file_of(s) * sqSize;
    int y = topLeft.y + relative_rank(~perspective, s) * sqSize;
    return {x, y};
}

bool PromotionSelector::contains(int x, int y) {
    return x > topLeft.x && x < topLeft.x + 4 * sqSize && y > topLeft.y && y < topLeft.y + sqSize;
}

Move PromotionSelector::move_on(int x, int y) {
    assert(contains(x, y));
    return moves[(x - topLeft.x) / sqSize];
}
