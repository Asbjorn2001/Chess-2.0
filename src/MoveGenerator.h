#include <array>
#include <cstdint>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "ChessBoard.h"

struct CheckData {
    SqOccPair checking_piece;
    std::vector<ChessSquare> resolve_squares;
};

struct MoveEvalData {
    ChessColor hostile_color{};
    std::array<uint8_t, 64> hostile_control_map{};
    std::vector<CheckData> hostile_checks{};
    std::map<SqOccPair, std::vector<ChessSquare>> hostile_pins{};

    MoveEvalData(ChessColor hostile_color = ChessColor::Neutral)
        : hostile_color(hostile_color) {};
};

using MoveMap = std::map<ChessSquare, std::vector<ChessMove>>;

struct PositionData {
    MoveMap legal_moves{};
    MoveEvalData eval_data{};
};

class MoveGenerator {
   public:
    MoveGenerator() = default;

    PositionData generate_position_data(const ChessBoard& board);

    MoveMap generate_pseudo_legal_moves(const ChessBoard& board);

   private:
    MoveEvalData generate_evaluation_data(
        const ChessBoard& board,
        std::vector<SqOccPair> hostile_occupants);

    std::vector<ChessMove> generate_sliding_moves(const ChessBoard& board,
                                                  SqOccPair piece);
    std::vector<ChessMove> generate_pawn_moves(const ChessBoard& board,
                                               SqOccPair pawn);
    std::vector<ChessMove> generate_knight_moves(const ChessBoard& board,
                                                 SqOccPair knight);
    std::vector<ChessMove> generate_king_moves(const ChessBoard& board,
                                               SqOccPair king);

    void generate_sliding_evaluation_data(const ChessBoard& board,
                                          MoveEvalData& data,
                                          SqOccPair sliding_piece);

    void generate_pawn_evaluation_data(const ChessBoard& board,
                                       MoveEvalData& data,
                                       SqOccPair pawn);

    void generate_knight_evaluation_data(const ChessBoard& board,
                                         MoveEvalData& data,
                                         SqOccPair knight);

    void generate_king_evaluation_data(const ChessBoard& board,
                                       MoveEvalData& data,
                                       SqOccPair king);

    bool is_check(const ChessBoard& board,
                  ChessSquare square,
                  SquareOccupant piece);
};
