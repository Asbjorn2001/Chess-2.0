#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <array>
#include <optional>
#include <utility>
#include <vector>
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

class PromotionSelector {
   public:
    PromotionSelector(SDL_Point topLeft, int sqSize, Square from, Square to)
        : topLeft(topLeft), sqSize(sqSize) {
        moves = {Move::make<PROMOTION>(from, to, KNIGHT), Move::make<PROMOTION>(from, to, BISHOP),
                 Move::make<PROMOTION>(from, to, ROOK), Move::make<PROMOTION>(from, to, QUEEN)};
    }

    bool contains(int x, int y);
    Move move_on(int x, int y);

    SDL_Point topLeft;
    int sqSize;

   private:
    std::array<Move, 4> moves;
};

class Board {
   public:
    Board(SDL_Point topLeft, int size) : topLeft(topLeft), size(size), sqSize(size / 8) {}

    /// Returns if the board contains the given coordinates. Calling this function with coordinates
    /// not contained by the board yields undefined behaviour.
    bool contains(int x, int y);
    /// Returns the square on the given coordinates.
    Square square_on(int x, int y, Color perspective);
    /// Returns the top left corner of the given square.
    SDL_Point corner_of(Square s, Color perspective);

    SDL_Point topLeft;
    int size;
    int sqSize;
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
    bool try_move(Move m);
    void select(Square s);
    void unselect();
    void open_selector(Square from, Square to);
    void close_selector();

    /// Returns the legal squares to move to from the given square.
    std::vector<Square> legal_squares_from(Square from) const;

    void update();
    void render();

    bool isRunning = true;
    int mouseX, mouseY;

    Board board;
    Position position{};
    Color perspective = WHITE;
    std::optional<Selected> selected = std::nullopt;
    std::optional<PromotionSelector> promotionSelector = std::nullopt;

    SDL_Window* window;
    SDL_Renderer* renderer;
};
