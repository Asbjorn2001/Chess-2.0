#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <array>
#include <cassert>
#include <string>
#include "types.h"

namespace Rendering {
constexpr SDL_Color TRANSPARENT = {0, 0, 0, 0};
constexpr SDL_Color RED = {255, 0, 0, 255};
constexpr SDL_Color GREEN = {0, 255, 0, 255};
constexpr SDL_Color BLUE = {0, 0, 255, 255};
constexpr SDL_Color DARK = {80, 55, 35, 255};
constexpr SDL_Color LIGHT = {190, 150, 120, 255};
constexpr SDL_Color BLACK = {0, 0, 0, 255};
constexpr SDL_Color WHITE = {255, 255, 255, 255};

extern std::array<SDL_Texture*, 12> PieceTextures;
extern std::array<SDL_Texture*, 8> RankLabels;
extern std::array<std::array<SDL_Texture*, 8>, 2> FileLabels;
extern TTF_Font* GoogleSans;

SDL_Texture* load_texture(SDL_Renderer* renderer, const std::string& path);

/// Must be called once at initialization.
void init(SDL_Renderer* renderer);
}  // namespace Rendering

/// Clears the renderer with the given color.
void clear(SDL_Renderer* renderer, const SDL_Color& color);

void draw_rect(SDL_Renderer* renderer, const SDL_Rect& rect, const SDL_Color& color);
void draw_square(SDL_Renderer* renderer,
                 const SDL_Point& topLeft,
                 int size,
                 const SDL_Color& color);
void draw_text(SDL_Renderer* renderer, const std::string& text);
void draw_texture(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect& dstRect);

/// Draw a single chess piece to the dstRect. `NO_PIECE` yields undefined behaviour.
void draw_piece(SDL_Renderer* renderer, Piece p, const SDL_Rect& dstRect);

/// Draws a chess board with the given board array, from a top left point and a size.
/// Default is to draw from white's perspective, but can be changed using the `perspective`
/// parameter.
void draw_board(SDL_Renderer* renderer,
                std::array<Piece, 64> board,
                const SDL_Point& topLeft,
                int size,
                const Color& perspective = WHITE);
