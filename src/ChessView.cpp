#include <SDL2/SDL.h>
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <array>
#include <cstdlib>
#include <format>
#include <stdexcept>
#include <string_view>
#include "ChessView.h"

constexpr SDL_Color cyan_t{0, 255, 255, 100};
constexpr SDL_Color blue_t{0, 0, 255, 100};
constexpr SDL_Color red_t{255, 0, 0, 100};
constexpr SDL_Color yellow_t{255, 255, 0, 100};
constexpr SDL_Color orange_t{255, 165, 0, 100};

Texture::Texture(SDL_Renderer* renderer, std::string_view path) {
    m_texture = IMG_LoadTexture(renderer, &path[0]);
    if (!m_texture) {
        throw std::logic_error(std::format("Could not create texture from: {}\n", path));
    }
}

Texture::~Texture() {
    SDL_DestroyTexture(m_texture);
}

void Texture::draw(SDL_Renderer* renderer, const SDL_Rect* rect) const {
    SDL_RenderCopy(renderer, m_texture, nullptr, rect);
}

ChessView::ChessView(SDL_Renderer* renderer, const ChessModel& model)
    : m_renderer(renderer), m_model(model) {
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    constexpr std::array<char, 12> piece_symbols{
        'p', 'P', 'b', 'B', 'n', 'N', 'r', 'R', 'q', 'Q', 'k', 'K',
    };

    for (const auto symbol : piece_symbols) {
        std::string path{std::format("assets/{}.png", symbol)};
        m_textures.insert_or_assign(symbol, Texture{m_renderer, path});
    }
}

void ChessView::draw() {
    draw_board();
    draw_selected();
    if (m_model.settings.debug_mode) {
        draw_eval_data(m_model.position_data.eval_data);
    }
    SDL_RenderPresent(m_renderer);
}

SDL_Rect ChessView::square_as_rect(const ChessSquare square) {
    const auto square_size = m_model.settings.square_size();
    return {
        m_model.settings.board_pos_x + square.col() * square_size,
        m_model.settings.board_pos_y + (7 - square.row()) * square_size,
        square_size,
        square_size,
    };
}

void ChessView::draw_square(const SDL_Rect* rect, const SDL_Color& color) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(m_renderer, rect);
}

void ChessView::draw_board() {
    SDL_Color light{100, 100, 100, 255};
    SDL_Color dark{200, 200, 200, 255};
    for (const auto rank : ranks) {
        for (const auto file : files) {
            const auto square{ChessSquare{file, rank}};
            const auto occ{m_model.board[square]};
            const auto rect = square_as_rect(square);

            if ((square.row() % 2 == 0 && square.col() % 2 == 0) ||
                (square.row() % 2 == 1 && square.col() % 2 == 1)) {
                draw_square(&rect, light);
            } else {
                draw_square(&rect, dark);
            }

            if (occ.is_piece()) {
                m_textures.at(static_cast<char>(occ)).draw(m_renderer, &rect);
            }
        }
    }
}

void ChessView::draw_selected() {
    if (m_model.selected_square.has_value()) {
        const auto square = m_model.selected_square.value();
        const auto rect = square_as_rect(square);
        draw_square(&rect, cyan_t);

        for (const auto& move : m_model.get_legal_moves_at(square)) {
            const auto rect{square_as_rect(move.target_square)};
            draw_square(&rect, cyan_t);
        }
    }
}

void ChessView::draw_eval_data(const MoveEvalData& data) {
    for (size_t sq{0}; sq < 64; ++sq) {
        if (data.hostile_control_map[sq] > 0) {
            const auto rect{square_as_rect(sq)};
            draw_square(&rect, red_t);
        }
    }

    for (const auto& [_, squares] : data.hostile_pins) {
        for (const auto& sq : squares) {
            const auto rect{square_as_rect(sq)};
            draw_square(&rect, yellow_t);
        }
    }

    for (const auto& check : data.hostile_checks) {
        for (const auto& sq : check.resolve_squares) {
            const auto rect{square_as_rect(sq)};
            draw_square(&rect, orange_t);
        }
    }
}
