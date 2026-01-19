#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <array>
#include <optional>
#include <utility>
#include "movegen.h"
#include "position.h"
#include "rendering.h"
#include "types.h"

constexpr int BOARD_MARGIN = 20;
constexpr int MIN_SIZE = 128;

struct Selected {
    Square square;
    Piece piece;
    bool stick = true;
};

class Board {
   public:
    Board(SDL_Point topLeft, int size) : topLeft(topLeft), size(size), squareSize(size / 8) {}

    /// Returns if the board contains the given coordinates. Calling this function with coordinates
    /// not contained by the board yields undefined behaviour.
    bool contains(int x, int y);
    /// Returns the square on the given coordinates.
    Square square_on(int x, int y, Color perspective);

    SDL_Point topLeft;
    int size;
    int squareSize;
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

    /// Checks if the move is legal, before making the move on the position.
    /// Returns true if the move was made.

    bool try_move(Square from, Square to);
    void select(Square s);
    void unselect();

    void update();
    void render();

    bool isRunning = true;
    int mouseX, mouseY;
    Board board;
    Position position{};
    Color perspective = WHITE;
    std::optional<Selected> selected = std::nullopt;

    SDL_Window* window;
    SDL_Renderer* renderer;
};
