#include "MoveGenerator.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

constexpr std::array<int, 8> direction_offsets = { 8, -8, 1, -1, 7, -7, 9, -9 };
constexpr size_t north = 0;
constexpr size_t south = 1;
constexpr size_t east = 2;
constexpr size_t west = 3;
constexpr size_t north_west = 4;
constexpr size_t south_east = 5;
constexpr size_t north_east = 6;
constexpr size_t south_west = 7;

constexpr std::array<std::array<uint8_t, 8>, 64> calc_num_squares_to_edge() {
    std::array<std::array<uint8_t, 8>, 64> num_squares {};
    for (size_t rank = 0; rank < 8; ++rank) {
        for (size_t file = 0; file < 8; ++file) {
            const auto num_north =  7 - rank;
            const auto num_south = rank;
            const auto num_east = 7 - file;
            const auto num_west = file;

            const auto square_index = rank * 8 + file;
            num_squares[square_index] = {
                static_cast<uint8_t>(num_north),
                static_cast<uint8_t>(num_south),
                static_cast<uint8_t>(num_east),
                static_cast<uint8_t>(num_west),
                static_cast<uint8_t>(std::min(num_north, num_west)),
                static_cast<uint8_t>(std::min(num_south, num_east)),
                static_cast<uint8_t>(std::min(num_north, num_east)),
                static_cast<uint8_t>(std::min(num_south, num_west)),
            };
        }
    }

    return num_squares;
}

constexpr std::array<std::array<uint8_t, 8>, 64> num_squares_to_edge = calc_num_squares_to_edge();

struct KnightMoves {
    std::array<uint8_t, 8> targets{};
    uint8_t count = 0;
};

constexpr std::array<KnightMoves, 64> calc_knight_squares() {
    constexpr std::array<std::pair<int, int>, 8> knight_deltas{{
        { 2,  1}, { 2, -1},
        {-2,  1}, {-2, -1},
        { 1,  2}, { 1, -2},
        {-1,  2}, {-1, -2}
    }};

    std::array<KnightMoves, 64> knight_squares;

    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            const auto square_index = static_cast<size_t>(rank * 8 + file);
            KnightMoves km {};
            for (const auto [dr, df] : knight_deltas) {
                const auto target_rank = rank + dr;
                const auto target_file = file + df;

                if (target_rank >= 0 && target_rank <= 7 && 
                    target_file >= 0 && target_file <= 7) 
                {
                    const auto target_index = target_rank * 8 + target_file;
                    km.targets[km.count++] = static_cast<uint8_t>(target_index); 
                    
                }            
            }
            knight_squares[square_index] = km;
        }
    }

    return knight_squares;
}

constexpr std::array<KnightMoves, 64> knight_squares = calc_knight_squares();


std::vector<ChessMove> MoveGenerator::generate_legal_moves(ChessBoard board) {
    std::vector<ChessMove> legal_moves = generate_pseudo_legal_moves(board);

    return legal_moves;
}


std::vector<ChessMove> MoveGenerator::generate_pseudo_legal_moves(const ChessBoard& board) {
    auto active_pieces = board.get_pieces(board.active_color());
    std::vector<ChessMove> moves {};

    for (const auto& [s, p] : active_pieces) {
        std::vector<ChessMove> moves_to_add{};
        switch (p.type()) {
            case OccupantType::Pawn:
                moves_to_add = generate_pawn_moves(board, s, p);
                break;
            case OccupantType::Knight:
                moves_to_add = generate_knight_moves(board, s, p);
                break;
            case OccupantType::King:
                moves_to_add = generate_king_moves(board, s, p);
                break;
            case OccupantType::Bishop:
            case OccupantType::Rook:
            case OccupantType::Queen:
                moves_to_add = generate_sliding_moves(board, s, p);
                break;
            default:
                break;
        }

        moves.insert(moves.end(), moves_to_add.begin(), moves_to_add.end());
    }

    return moves;
}

std::vector<ChessMove> MoveGenerator::generate_sliding_moves(const ChessBoard& board, ChessSquare start_square, SquareOccupant piece) {
    const ChessColor friendly_color = piece.piece_color();
    std::vector<ChessMove> moves {};
    size_t b_index {0}, e_index {8};
    if (piece.type() == OccupantType::Rook) e_index = 4;
    if (piece.type() == OccupantType::Bishop) b_index = 4;

    for (size_t direction_index = b_index; direction_index < e_index; ++direction_index) {
        for (int n = 0; n < num_squares_to_edge[start_square][direction_index]; ++n) {
            const auto offset = direction_offsets[direction_index] * (n + 1);
            const ChessSquare target_square = static_cast<size_t>(static_cast<int>(start_square) + offset);
            const SquareOccupant target = board[target_square];
            if (target.is_piece()) {
                if (target.piece_color() != friendly_color) {
                    moves.push_back({ start_square,  target_square , true });
                }             
                break;
            }
            
            moves.push_back({ start_square, { target_square } });
        }
    }

    return moves;
}

std::vector<ChessMove> generate_pawn_moves(const ChessBoard& board, ChessSquare start_square, SquareOccupant pawn) {
    std::vector<ChessMove> moves {};
    const auto move_index = pawn.piece_color() == ChessColor::White ? north : south;
    const auto capture_indices = pawn.piece_color() == ChessColor::White ? 
        std::array<size_t, 2>{ north_west, north_east } 
        : std::array<size_t, 2>{ south_west, south_east };

    auto not_moved = 
        ((pawn.piece_color() == ChessColor::White && start_square.row() == 1) 
        || (pawn.piece_color() == ChessColor::Black && start_square.row() == 6));
    const size_t num_moves = not_moved ? 2 : 1;
    for (size_t i = 0; i < num_moves; ++i) {
        const auto offset = direction_offsets[move_index];
        const ChessSquare target_square = static_cast<size_t>(static_cast<int>(start_square) + offset);
        const SquareOccupant target = board[target_square];
        if (target.is_piece()) break;
        moves.push_back({ start_square, target_square });
    }

    for (auto capture_idx : capture_indices) {
        if (num_squares_to_edge[start_square.index()][capture_idx] > 0) {
            const auto offset = direction_offsets[capture_idx];
            const ChessSquare target_square = static_cast<size_t>(static_cast<int>(start_square) + offset);
            const SquareOccupant target = board[target_square];
            if (target.is_piece() && target.piece_color() != pawn.piece_color()) {
                moves.push_back({ start_square, target_square , true });
            }
        }
    }

    return moves;
}

std::vector<ChessMove> generate_knight_moves(const ChessBoard& board, ChessSquare start_square, SquareOccupant knight) {
    std::vector<ChessMove> moves{};
    const auto& knight_moves = knight_squares[start_square];
    for (size_t i = 0; i < knight_moves.count; ++i) {
        const ChessSquare target_square = knight_moves.targets[i];
        const SquareOccupant target = board[target_square];
        if (target.is_piece()) {
            if (target.piece_color() != knight.piece_color()) {
                moves.push_back({ start_square, target_square, true });
            }
        } else {
            moves.push_back({ start_square, target_square });
        }
    }

    return moves;
}

std::vector<ChessMove> generate_king_moves(const ChessBoard& board, ChessSquare start_square, SquareOccupant king) {
    std::vector<ChessMove> moves{};
    for (size_t direction_index = 0; direction_index < 8; ++direction_index) {
        if (num_squares_to_edge[start_square][direction_index] == 0) {
            continue;
        }
        const auto offset = direction_offsets[direction_index];
        const ChessSquare target_square = static_cast<size_t>(static_cast<int>(start_square) + offset);
        const SquareOccupant target = board[target_square];
        if (target.is_piece()) {
            if (target.piece_color() != king.piece_color()) {
                moves.push_back({ start_square, target_square, true });
            }
        } else {
            moves.push_back({ start_square, target_square });
        }
    }

    return moves;
}
