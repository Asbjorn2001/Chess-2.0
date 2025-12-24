#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ranges>
#include <set>
#include <utility>
#include <vector>

#include "MoveGenerator.h"

constexpr std::array<int, 8> direction_offsets = {8, -8, 1, -1, 7, -7, 9, -9};
constexpr size_t north = 0;
constexpr size_t south = 1;
constexpr size_t east = 2;
constexpr size_t west = 3;
constexpr size_t north_west = 4;
constexpr size_t south_east = 5;
constexpr size_t north_east = 6;
constexpr size_t south_west = 7;

constexpr std::array<std::array<uint8_t, 8>, 64> calc_num_squares_to_edge() {
    std::array<std::array<uint8_t, 8>, 64> num_squares{};
    for (size_t rank{0}; rank < 8; ++rank) {
        for (size_t file{0}; file < 8; ++file) {
            const auto num_north{7 - rank};
            const auto num_south{rank};
            const auto num_east{7 - file};
            const auto num_west{file};

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

constexpr std::array<std::array<uint8_t, 8>, 64> num_squares_to_edge =
    calc_num_squares_to_edge();

struct KnightMoves {
    std::array<uint8_t, 8> targets{};
    uint8_t count = 0;
};

constexpr std::array<KnightMoves, 64> calc_knight_squares() {
    constexpr std::array<std::pair<int, int>, 8> knight_deltas{{{2, 1},
                                                                {2, -1},
                                                                {-2, 1},
                                                                {-2, -1},
                                                                {1, 2},
                                                                {1, -2},
                                                                {-1, 2},
                                                                {-1, -2}}};

    std::array<KnightMoves, 64> knight_squares;

    for (size_t rank{0}; rank < 8; ++rank) {
        for (size_t file{0}; file < 8; ++file) {
            const auto square_index{rank * 8 + file};
            KnightMoves knight_moves{};
            for (const auto [dr, df] : knight_deltas) {
                const auto target_rank = static_cast<int>(rank) + dr;
                const auto target_file = static_cast<int>(file) + df;

                if (target_rank >= 0 && target_rank <= 7 && target_file >= 0 &&
                    target_file <= 7) {
                    const auto target_index{target_rank * 8 + target_file};
                    knight_moves.targets[knight_moves.count++] =
                        static_cast<uint8_t>(target_index);
                }
            }
            knight_squares[square_index] = knight_moves;
        }
    }

    return knight_squares;
}

constexpr std::array<KnightMoves, 64> knight_squares = calc_knight_squares();

std::vector<ChessMove> MoveGenerator::generate_legal_moves(ChessBoard board) {
    std::vector<ChessMove> legal_moves = generate_pseudo_legal_moves(board);

    auto all_pieces = board.get_pieces(ChessColor::Neutral);
    auto active_color = board.active_color();

    std::vector<std::pair<ChessSquare, SquareOccupant>> active_pieces{};
    std::vector<std::pair<ChessSquare, SquareOccupant>> opponent_pieces{};

    for (const auto& [square, piece] : all_pieces) {
        if (piece.piece_color() == active_color) {
        }
    }

    return legal_moves;
}

std::vector<ChessMove> MoveGenerator::generate_pseudo_legal_moves(
    const ChessBoard& board) {
    const auto active_pieces = board.get_pieces(board.active_color());
    std::vector<ChessMove> moves{};

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

std::vector<ChessSquare> generate_sliding_target_squares(
    ChessSquare start_square,
    SquareOccupant piece) {
    std::vector<ChessSquare> squares{};
    size_t b_index{0}, e_index{8};
    if (piece.type() == OccupantType::Rook)
        e_index = 4;
    if (piece.type() == OccupantType::Bishop)
        b_index = 4;

    for (size_t direction_index = b_index; direction_index < e_index;
         ++direction_index) {
        for (auto n{0}; n < num_squares_to_edge[start_square][direction_index];
             ++n) {
            const auto offset = direction_offsets[direction_index] * (n + 1);
            const ChessSquare target_square = start_square.offset_with(offset);
            squares.push_back(target_square);
        }
    }

    return squares;
}

std::vector<ChessMove> MoveGenerator::generate_sliding_moves(
    const ChessBoard& board,
    ChessSquare start_square,
    SquareOccupant piece) {
    const ChessColor friendly_color = piece.piece_color();
    std::vector<ChessMove> moves{};

    for (const auto& target_square :
         generate_sliding_target_squares(start_square, piece)) {
        const SquareOccupant target{board[target_square]};
        if (target.is_piece()) {
            if (target.piece_color() != friendly_color) {
                moves.push_back({
                    .start_square = start_square,
                    .target_square = target_square,
                    .capture = {{target_square, target}},
                });
            }
            break;
        }

        moves.push_back({start_square, {target_square}});
    }

    return moves;
}

std::vector<ChessSquare> generate_sliding_control_squares(
    const ChessBoard& board,
    ChessSquare start_square,
    SquareOccupant piece) {
    std::vector<ChessSquare> squares{};
    for (const auto& target_square :
         generate_sliding_target_squares(start_square, piece)) {
        squares.push_back(target_square);
        if (board[target_square].is_piece()) {
            break;
        }
    }

    return squares;
}

std::vector<ChessSquare> generate_pawn_control_squares(ChessSquare start_square,
                                                       SquareOccupant pawn) {
    std::vector<ChessSquare> squares{};

    const auto capture_indices =
        pawn.piece_color() == ChessColor::White
            ? std::array<size_t, 2>{north_west, north_east}
            : std::array<size_t, 2>{south_west, south_east};

    for (auto capture_index : capture_indices) {
        if (num_squares_to_edge[start_square][capture_index] > 0) {
            squares.push_back(
                start_square.offset_with(direction_offsets[capture_index]));
        }
    }

    return squares;
}

std::vector<ChessMove> MoveGenerator::generate_pawn_moves(
    const ChessBoard& board,
    ChessSquare start_square,
    SquareOccupant pawn) {
    std::vector<ChessMove> moves{};
    const auto move_index{pawn.piece_color() == ChessColor::White ? north
                                                                  : south};
    const auto move_offset{direction_offsets[move_index]};

    const bool first_move{
        ((pawn.piece_color() == ChessColor::White && start_square.row() == 1) ||
         (pawn.piece_color() == ChessColor::Black && start_square.row() == 6))};

    const int num_moves{first_move ? 2 : 1};
    for (int n{0}; n < num_moves; ++n) {
        const ChessSquare target_square{
            start_square.offset_with(move_offset * (n + 1))};
        const SquareOccupant target{board[target_square]};
        if (target.is_piece())
            break;

        if (n == 1) {
            moves.push_back({
                .start_square = start_square,
                .target_square = target_square,
                .en_passant_target = target_square.offset_with(-move_offset),
            });
        } else {
            moves.push_back({start_square, target_square});
        }
    }

    // Calculate capture moves
    const auto en_passant_target{board.en_pessant_target()};
    for (const auto& target_square :
         generate_pawn_control_squares(start_square, pawn)) {
        const SquareOccupant target{board[target_square]};
        if (target.is_piece() && target.piece_color() != pawn.piece_color()) {
            moves.push_back({
                .start_square = start_square,
                .target_square = target_square,
                .capture = {{target_square, target}},
            });
        } else if (en_passant_target && en_passant_target == target_square) {
            moves.push_back({
                .start_square = start_square,
                .target_square = target_square,
                .capture = {{target_square.offset_with(-move_offset), target}},
            });
        }
    }

    return moves;
}

std::vector<ChessSquare> generate_knight_control_squares(
    ChessSquare start_square) {
    std::vector<ChessSquare> squares{};
    const auto& knight_moves = knight_squares[start_square];
    for (size_t i{0}; i < knight_moves.count; ++i) {
        squares.push_back(knight_moves.targets[i]);
    }
    return squares;
}

std::vector<ChessMove> MoveGenerator::generate_knight_moves(
    const ChessBoard& board,
    ChessSquare start_square,
    SquareOccupant knight) {
    std::vector<ChessMove> moves{};
    for (auto& target_square : generate_knight_control_squares(start_square)) {
        const SquareOccupant target{board[target_square]};
        if (target.is_piece()) {
            if (target.piece_color() != knight.piece_color()) {
                moves.push_back({
                    .start_square = start_square,
                    .target_square = target_square,
                    .capture = {{target_square, target}},
                });
            }
        } else {
            moves.push_back({start_square, target_square});
        }
    }

    return moves;
}

std::vector<ChessSquare> generate_king_control_squares(
    ChessSquare start_square) {
    std::vector<ChessSquare> squares{};
    for (size_t direction_index{0}; direction_index < 8; ++direction_index) {
        if (num_squares_to_edge[start_square][direction_index] == 0) {
            continue;
        }
        const auto offset = direction_offsets[direction_index];
        squares.push_back(start_square.offset_with(offset));
    }

    return squares;
}

std::vector<ChessMove> MoveGenerator::generate_king_moves(
    const ChessBoard& board,
    ChessSquare start_square,
    SquareOccupant king) {
    std::vector<ChessMove> moves{};
    for (const auto& target_square :
         generate_king_control_squares(start_square)) {
        const SquareOccupant target{board[target_square]};
        if (target.is_piece()) {
            if (target.piece_color() != king.piece_color()) {
                moves.push_back({
                    .start_square = start_square,
                    .target_square = target_square,
                    .capture = {{target_square, target}},
                });
            }
        } else {
            moves.push_back({start_square, target_square});
        }
    }

    return moves;
}

std::vector<ChessSquare> generate_control_squares(
    const ChessBoard& board,
    std::vector<SqOccPair> pieces) {
    std::set<ChessSquare> control_squares{};
    for (const auto& [square, piece] : pieces) {
        std::vector<ChessSquare> squares_to_add{};
        switch (piece.type()) {
            case OccupantType::Pawn:
                squares_to_add = generate_pawn_control_squares(square, piece);
                break;
            case OccupantType::Knight:
                squares_to_add = generate_knight_control_squares(square);
                break;
            case OccupantType::King:
                squares_to_add = generate_king_control_squares(square);
                break;
            case OccupantType::Bishop:
            case OccupantType::Rook:
            case OccupantType::Queen:
                squares_to_add =
                    generate_sliding_control_squares(board, square, piece);
                break;
            default:
                break;
        }
        control_squares.insert(squares_to_add.begin(), squares_to_add.end());
    }

    return std::vector(control_squares.begin(), control_squares.end());
}

MoveEvalData MoveGenerator::generate_evaluation_data(
    const ChessBoard& board,
    std::vector<SqOccPair> hostile_occupants) {
    MoveEvalData data{};

    for (const auto& [sq, occ] : hostile_occupants) {
        switch (occ.type()) {
            case OccupantType::Pawn:
                generate_pawn_evaluation_data(board, data, sq);
                break;
            case OccupantType::Knight:
                generate_knight_evaluation_data(board, data, sq);
                break;
            case OccupantType::King:
                generate_king_evaluation_data(board, data, sq);
                break;
            case OccupantType::Bishop:
            case OccupantType::Rook:
            case OccupantType::Queen:
                generate_sliding_evaluation_data(board, data, sq);
                break;
            default:
                break;
        }
    }
}

void MoveGenerator::generate_sliding_evaluation_data(const ChessBoard& board,
                                                     MoveEvalData& data,
                                                     ChessSquare square) {}

void MoveGenerator::generate_pawn_evaluation_data(const ChessBoard& board,
                                                  MoveEvalData& data,
                                                  ChessSquare square) {}

void MoveGenerator::generate_knight_evaluation_data(const ChessBoard& board,
                                                    MoveEvalData& data,
                                                    ChessSquare square) {}

void MoveGenerator::generate_king_evaluation_data(const ChessBoard& board,
                                                  MoveEvalData& data,
                                                  ChessSquare square) {}

bool is_check(const ChessBoard& board,
              ChessSquare square,
              SquareOccupant piece) {
    switch (piece.type()) {
        case OccupantType::Pawn:
            break;
        default:

            break;
    }
}
