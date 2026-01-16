#include <SDL2/SDL_image.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <cassert>
#include <cstddef>
#include <format>
#include <stdexcept>
#include <string>
#include "rendering.h"
#include "types.h"

constexpr SDL_Color board_color_of(Square s) {
    return (rank_of(s) % 2 == file_of(s) % 2) ? Rendering::DARK : Rendering::LIGHT;
}

namespace Rendering {
std::array<SDL_Texture*, 12> PieceTextures;
std::array<SDL_Texture*, 8> RankLabels;
std::array<std::array<SDL_Texture*, 8>, 2> FileLabels;
TTF_Font* GoogleSans;

SDL_Texture* load_texture(SDL_Renderer* renderer, const std::string& path) {
    SDL_Texture* texture = IMG_LoadTexture(renderer, &path[0]);
    if (!texture)
        throw std::logic_error(std::format("Could not create texture from: {}\n", path));
    return texture;
}

void init(SDL_Renderer* renderer) {
    // Load piece textures into memory.
    for (size_t i = 0; i < PieceTextures.size(); ++i) {
        std::string path = std::format("assets/{}.png", PieceChars[i]);
        PieceTextures[i] = load_texture(renderer, path);
    }

    if (TTF_Init() != 0)
        throw std::logic_error("Failed to initialize ttf.");

    // Load font into memory.
    GoogleSans = TTF_OpenFont("assets/GoogleSans-Regular.ttf", 24);

    // Load file and rank labels.
    for (int i = 0; i < 8; ++i) {
        const SDL_Color c1 = i % 2 == 0 ? LIGHT : DARK;
        const SDL_Color c2 = i % 2 == 0 ? DARK : LIGHT;
        auto rankSurface = TTF_RenderText_Blended(GoogleSans, std::to_string(i + 1).c_str(), c1);

        const char fStr[] = {char('a' + i)};
        auto fileWhitePerspSurface = TTF_RenderText_Blended(GoogleSans, &fStr[0], c1);
        auto fileBlackPerspSurface = TTF_RenderText_Blended(GoogleSans, &fStr[0], c2);

        RankLabels[i] = SDL_CreateTextureFromSurface(renderer, rankSurface);
        FileLabels[Color::WHITE][i] = SDL_CreateTextureFromSurface(renderer, fileWhitePerspSurface);
        FileLabels[Color::BLACK][i] = SDL_CreateTextureFromSurface(renderer, fileBlackPerspSurface);

        SDL_FreeSurface(rankSurface);
        SDL_FreeSurface(fileWhitePerspSurface);
        SDL_FreeSurface(fileBlackPerspSurface);
    }
}
}  // namespace Rendering

void clear(SDL_Renderer* renderer, const SDL_Color& color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer);
}

void draw_rect(SDL_Renderer* renderer, const SDL_Rect& rect, const SDL_Color& color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

void draw_square(SDL_Renderer* renderer,
                 const SDL_Point& topLeft,
                 int size,
                 const SDL_Color& color) {
    SDL_Rect square = {topLeft.x, topLeft.y, size, size};
    draw_rect(renderer, square, color);
}

void draw_texture(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect& dstRect) {
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
}

void draw_piece(SDL_Renderer* renderer, Piece p, const SDL_Rect& dstRect) {
    assert(p != NO_PIECE);
    draw_texture(renderer, Rendering::PieceTextures[pc_as_index(p)], dstRect);
}

void draw_board(SDL_Renderer* renderer,
                std::array<Piece, 64> board,
                const SDL_Point& topLeft,
                int size,
                const Color& perspective) {
    int squareSize = size / 8;
    int fourthSS = squareSize / 4;
    for (Square s = SQ_A1; s < SQUARE_NB; ++s) {
        int x = topLeft.x + file_of(s) * squareSize;
        int y = topLeft.y + relative_rank(~perspective, rank_of(s)) * squareSize;

        // Draw the square background color
        SDL_Rect dstSquare = {x, y, squareSize, squareSize};
        draw_rect(renderer, dstSquare, board_color_of(s));

        // Draw square labels, if they are on file a or relative rank 1
        if (relative_rank(perspective, rank_of(s)) == RANK_1)
            draw_texture(renderer, Rendering::FileLabels[perspective][file_of(s)],
                         {dstSquare.x + 7 * (fourthSS / 2), dstSquare.y + 3 * fourthSS,
                          fourthSS / 2, fourthSS});
        if (file_of(s) == FILE_A)
            draw_texture(renderer, Rendering::RankLabels[rank_of(s)],
                         {dstSquare.x, dstSquare.y, fourthSS / 2, fourthSS});

        // Draw piece if any
        if (Piece p = board[s]; p != NO_PIECE)
            draw_piece(renderer, p, dstSquare);
    }
}
