#include <map>
#include <set>
#include <utility>
#include <vector>

#include "ChessBoard.h"

using SqOccPair = std::pair<ChessSquare, SquareOccupant>;

struct PieceControlSquares {
    SqOccPair piece;
    std::set<ChessSquare> control_squares;
};

struct CheckData {
    SqOccPair checking_piece;
    std::vector<ChessSquare> resolve_squares;
};

// A pin is: [Location of pinned piece, A list of squares that they are allowed
// to move to]
using Pins = std::map<ChessSquare, std::vector<ChessSquare>>;

struct MoveEvalData {
    std::set<ChessSquare> hostile_control_squares;
    std::vector<CheckData> checks;
    Pins pins;

    MoveEvalData();
};

class MoveGenerator {
   public:
    MoveGenerator() = default;

    std::vector<ChessMove> generate_legal_moves(ChessBoard board);

   private:
    std::vector<ChessMove> generate_pseudo_legal_moves(const ChessBoard& board);
    std::vector<ChessMove> generate_sliding_moves(const ChessBoard& board,
                                                  ChessSquare start_square,
                                                  SquareOccupant piece);
    std::vector<ChessMove> generate_pawn_moves(const ChessBoard& board,
                                               ChessSquare start_square,
                                               SquareOccupant pawn);
    std::vector<ChessMove> generate_knight_moves(const ChessBoard& board,
                                                 ChessSquare start_square,
                                                 SquareOccupant knight);
    std::vector<ChessMove> generate_king_moves(const ChessBoard& board,
                                               ChessSquare start_square,
                                               SquareOccupant king);

    MoveEvalData generate_evaluation_data(
        const ChessBoard& board,
        std::vector<SqOccPair> hostile_occupants);

    void generate_sliding_evaluation_data(const ChessBoard& board,
                                          MoveEvalData& data,
                                          ChessSquare square);

    void generate_pawn_evaluation_data(const ChessBoard& board,
                                       MoveEvalData& data,
                                       ChessSquare square);

    void generate_knight_evaluation_data(const ChessBoard& board,
                                         MoveEvalData& data,
                                         ChessSquare square);

    void generate_king_evaluation_data(const ChessBoard& board,
                                       MoveEvalData& data,
                                       ChessSquare square);

    bool is_check(const ChessBoard& board,
                  ChessSquare square,
                  SquareOccupant piece);
};
