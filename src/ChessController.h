#include <SDL2/SDL_events.h>
#include <optional>
#include <unordered_map>
#include <vector>
#include "MoveGenerator.h"

struct Settings {
    int board_size;
    int board_pos_x;
    int board_pos_y;
    bool debug_mode;

    constexpr int square_size() const { return board_size / 8; }
};

struct ChessModel {
    ChessBoard board;
    bool is_check;
    bool is_checkmate;
    std::optional<ChessSquare> selected_square;
    PositionData position_data;
    Settings settings;

    ChessModel(ChessBoard board, Settings settings);

    std::vector<ChessMove> get_legal_moves_at(ChessSquare square) const;
};

class ChessController {
   public:
    ChessController(ChessModel& model);
    ChessController(ChessController&&) = default;
    ChessController(const ChessController&) = default;
    ChessController& operator=(ChessController&&) = delete;
    ChessController& operator=(const ChessController&) = delete;
    ~ChessController() = default;

    void handle_event(SDL_Event event);

   private:
    ChessModel& m_model;
    MoveGenerator m_move_generator;

    std::optional<ChessSquare> get_square_from(int mouse_x, int mouse_y);
    bool try_select(ChessSquare square);
    bool try_move(ChessSquare start_square, ChessSquare target_square);
    void make_move(const ChessMove& move);
};
