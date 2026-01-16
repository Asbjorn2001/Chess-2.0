#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <array>
#include <optional>
#include <utility>
#include "position.h"
#include "rendering.h"
#include "types.h"

constexpr int BOARD_MARGIN = 20;
constexpr int MIN_SIZE = 128;

struct Selected {
    Square square;
    Piece piece;
    int mouseX;
    int mouseY;
    bool stick = true;
};

class Board {
   public:
    Board(SDL_Point topLeft, int size, Color perspective = WHITE)
        : topLeft(topLeft), size(size), squareSize(size / 8), perspective(perspective) {}

    /// Returns if the board contains the given coordinates. Calling this function with coordinates
    /// not contained by the board yields undefined behaviour.
    bool contains(int x, int y);
    /// Returns the square on the given coordinates.
    Square square_on(int x, int y);
    void switch_perspective();
    void handle_press(SDL_MouseButtonEvent e);
    void handle_release(SDL_MouseButtonEvent e);
    void handle_motion(SDL_MouseMotionEvent e);
    void draw(SDL_Renderer* renderer);

   private:
    SDL_Point topLeft;
    int size;
    int squareSize;
    Color perspective;
    std::optional<Selected> selected = std::nullopt;
    Position position{};
};

class ChessGUI {
   public:
    ChessGUI(int size = 640);
    ~ChessGUI();
    ChessGUI(const ChessGUI&) = delete;
    ChessGUI& operator=(const ChessGUI&) = delete;
    ChessGUI(ChessGUI&&) = delete;
    ChessGUI& operator=(ChessGUI&&) = delete;
    void loop();

   private:
    /// High level event handler.
    void handle_event(SDL_Event e);
    void handle_press(SDL_MouseButtonEvent e);
    void handle_release(SDL_MouseButtonEvent e);
    void handle_motion(SDL_MouseMotionEvent e);

    void update();
    void render();

    bool isRunning = true;
    Position position;
    Board board;
    std::optional<Selected> selected;
    SDL_Window* window;
    SDL_Renderer* renderer;
};

inline void Board::switch_perspective() {
    perspective = ~perspective;
}
