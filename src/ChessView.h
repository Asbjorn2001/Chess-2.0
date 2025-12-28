#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <memory>
#include <string_view>
#include <unordered_map>
#include "ChessController.h"

class ChessTexture {
   public:
    ChessTexture(SDL_Renderer* renderer, std::string_view path);
    ChessTexture(const ChessTexture&) = delete;
    ChessTexture& operator=(const ChessTexture&) = delete;
    ChessTexture(ChessTexture&& rhs) noexcept : m_texture(rhs.m_texture) {
        rhs.m_texture = nullptr;
    };
    ChessTexture& operator=(ChessTexture&& rhs) noexcept {
        if (this != &rhs) {
            SDL_DestroyTexture(m_texture);
            m_texture = rhs.m_texture;
            rhs.m_texture = nullptr;
        }
        return *this;
    }
    ~ChessTexture();

    void draw(SDL_Renderer* renderer, const SDL_Rect* rect) const;

   private:
    SDL_Texture* m_texture;
};

class ChessView {
   public:
    ChessView(SDL_Renderer* renderer, const ChessModel& model);
    ChessView(ChessView&&) = default;
    ChessView(const ChessView&) = default;
    ChessView& operator=(ChessView&&) = delete;
    ChessView& operator=(const ChessView&) = delete;

    void draw();

   private:
    SDL_Rect square_as_rect(const ChessSquare square);
    void draw_square(const SDL_Rect* rect, const SDL_Color& color);
    void draw_board();
    void draw_selected();
    void draw_eval_data(const MoveEvalData& data);

    SDL_Renderer* m_renderer;
    const ChessModel& m_model;
    std::unordered_map<char, ChessTexture> m_textures;
};
