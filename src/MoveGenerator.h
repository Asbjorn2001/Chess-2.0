#include <vector>
#include "ChessBoard.h"

class MoveGenerator {
public:
    MoveGenerator();

    std::vector<ChessMove> generate_legal_moves(ChessBoard board);

private:
    std::vector<ChessMove> generate_pseudo_legal_moves(const ChessBoard& board);
    std::vector<ChessMove> generate_sliding_moves(const ChessBoard& board, ChessSquare start_square, SquareOccupant piece);
    std::vector<ChessMove> generate_pawn_moves(const ChessBoard& board, ChessSquare start_square, SquareOccupant pawn);
    std::vector<ChessMove> generate_knight_moves(const ChessBoard& board, ChessSquare start_square, SquareOccupant knight);
    std::vector<ChessMove> generate_king_moves(const ChessBoard& board, ChessSquare start_square, SquareOccupant king);

    std::vector<ChessSquare> attacked_squares;
    std::vector<ChessSquare> pinned_squares;
    std::vector<ChessSquare> checked_squares;
};
