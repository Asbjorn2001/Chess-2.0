#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ranges>
#include <set>
#include <stdexcept>
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
    std::array<uint8_t, 8> squares{};
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
                    knight_moves.squares[knight_moves.count++] =
                        static_cast<uint8_t>(target_index);
                }
            }
            knight_squares[square_index] = knight_moves;
        }
    }

    return knight_squares;
}

constexpr std::array<KnightMoves, 64> knight_squares = calc_knight_squares();

auto sliding_piece_rays(ChessSquare start_square, OccupantType piece_type) {
    auto directions =
        (piece_type == OccupantType::Rook)     ? std::views::iota(0u, 4u)
        : (piece_type == OccupantType::Bishop) ? std::views::iota(4u, 8u)
                                               : std::views::iota(0u, 8u);

    return directions | std::views::transform([sq = start_square](size_t dir) {
               return std::views::iota(1, num_squares_to_edge[sq][dir] + 1) |
                      std::views::transform([=](int n) {
                          return sq.offset_with(direction_offsets[dir] * n);
                      });
           });
}

auto pawn_control_squares(ChessSquare start_square, ChessColor pawn_color) {
    static constexpr std::array<size_t, 2> white_dirs{north_west, north_east};
    static constexpr std::array<size_t, 2> black_dirs{south_west, south_east};

    const auto& capture_dirs =
        pawn_color == ChessColor::White ? white_dirs : black_dirs;

    return capture_dirs | std::views::filter([sq = start_square](size_t dir) {
               return num_squares_to_edge[sq][dir] > 0;
           }) |
           std::views::transform([sq = start_square](size_t dir) {
               return sq.offset_with(direction_offsets[dir]);
           });
}

auto knight_control_squares(ChessSquare start_square) {
    const auto& knight_moves = knight_squares[start_square];
    return std::views::iota(0u, knight_moves.count) |
           std::views::transform([squares = knight_moves.squares](size_t i) {
               return static_cast<size_t>(squares[i]);
           });
}

auto king_control_squares(ChessSquare start_square) {
    return std::views::iota(0u, 8u) |
           std::views::filter([sq = start_square](size_t dir) {
               return num_squares_to_edge[sq][dir] > 0;
           }) |
           std::views::transform([sq = start_square](size_t dir) {
               return sq.offset_with(direction_offsets[dir]);
           });
}

bool sliding_piece_intersects_square(SqOccPair piece, ChessSquare square) {
    if (square == piece.square) {
        return false;
    }
    // Diagonal diffs are used to check if the squares are on the same diagonal
    const bool same_diag{square.row() - square.col() ==
                         piece.square.row() - piece.square.col()};
    const bool same_file{square.file() == piece.square.file()};
    const bool same_rank{square.rank() == piece.square.rank()};
    switch (piece.occ.type()) {
        case OccupantType::Bishop:
            return same_diag;
        case OccupantType::Rook:
            return same_file || same_rank;
        case OccupantType::Queen:
            return same_file || same_rank || same_diag;
        default:
            return false;
    }
}

std::vector<ChessMove> filter_moves(
    const std::vector<ChessMove>& pseudo_legal_moves,
    SqOccPair piece,
    const MoveEvalData& data) {
    std::vector<ChessMove> filtered_moves{};
    for (const auto& move : pseudo_legal_moves) {
        if (piece.occ.type() == OccupantType::King) {
            bool can_add{data.hostile_control_map[move.target_square] == 0};

            if (can_add) {
                can_add = !std::any_of(
                    data.hostile_checks.begin(), data.hostile_checks.end(),
                    [&](CheckData check) {
                        return check.checking_piece.occ.is_sliding_piece() &&
                               sliding_piece_intersects_square(
                                   check.checking_piece, move.target_square);
                    });
            }

            if (can_add && move.castle_move.has_value()) {
                const auto castle_move = move.castle_move.value();
                if (!data.hostile_checks.empty() ||
                    data.hostile_control_map[castle_move.king_target] > 0 ||
                    data.hostile_control_map[castle_move.rook_target] > 0) {
                    can_add = false;
                }
            }

            if (can_add) {
                filtered_moves.push_back(move);
            }
            continue;
        }

        bool can_add = true;
        if (!data.hostile_checks.empty()) {
            can_add = false;
            for (const auto& resolve_sq :
                 data.hostile_checks.front().resolve_squares) {
                if (resolve_sq == move.target_square) {
                    can_add = true;
                }
            }
        }

        if (can_add && data.hostile_pins.contains(piece)) {
            can_add = false;
            for (const auto& pin_sq : data.hostile_pins.at(piece)) {
                if (move.target_square == pin_sq) {
                    can_add = true;
                }
            }
        }
        if (can_add) {
            filtered_moves.push_back(move);
        }
    }

    return filtered_moves;
}

PositionData MoveGenerator::generate_position_data(const ChessBoard& board) {
    MoveMap legal_moves{};

    std::vector<SqOccPair> active_pieces{
        board.get_pieces(board.active_color())};
    std::vector<SqOccPair> hostile_pieces{
        board.get_pieces(board.passive_color())};

    const auto data = generate_evaluation_data(board, hostile_pieces);

    bool is_doublecheck = data.hostile_checks.size() > 1;
    for (const auto& piece : active_pieces) {
        if (is_doublecheck && piece.occ.type() != OccupantType::King) {
            continue;
        }
        std::vector<ChessMove> moves_to_add{};
        switch (piece.occ.type()) {
            case OccupantType::Pawn:
                moves_to_add = filter_moves(generate_pawn_moves(board, piece),
                                            piece, data);
                break;
            case OccupantType::Knight:
                moves_to_add = filter_moves(generate_knight_moves(board, piece),
                                            piece, data);
                break;
            case OccupantType::King:
                moves_to_add = filter_moves(generate_king_moves(board, piece),
                                            piece, data);
                break;
            case OccupantType::Bishop:
            case OccupantType::Rook:
            case OccupantType::Queen:
                moves_to_add = filter_moves(
                    generate_sliding_moves(board, piece), piece, data);
                break;
            default:
                break;
        }
        legal_moves.insert_or_assign(piece.square, moves_to_add);
    }

    return {legal_moves, data};
}

MoveMap MoveGenerator::generate_pseudo_legal_moves(const ChessBoard& board) {
    const auto active_pieces = board.get_pieces(board.active_color());
    MoveMap moves{};

    for (const auto& piece : active_pieces) {
        std::vector<ChessMove> moves_to_add{};
        switch (piece.occ.type()) {
            case OccupantType::Pawn:
                moves_to_add = generate_pawn_moves(board, piece);
                break;
            case OccupantType::Knight:
                moves_to_add = generate_knight_moves(board, piece);
                break;
            case OccupantType::King:
                moves_to_add = generate_king_moves(board, piece);
                break;
            case OccupantType::Bishop:
            case OccupantType::Rook:
            case OccupantType::Queen:
                moves_to_add = generate_sliding_moves(board, piece);
                break;
            default:
                break;
        }

        moves.insert_or_assign(piece.square, moves_to_add);
    }

    return moves;
}

std::vector<ChessMove> MoveGenerator::generate_sliding_moves(
    const ChessBoard& board,
    SqOccPair piece) {
    const ChessColor friendly_color = piece.occ.piece_color();
    std::vector<ChessMove> moves{};

    for (const auto& ray : sliding_piece_rays(piece.square, piece.occ.type())) {
        for (ChessSquare sq : ray) {
            const SquareOccupant occ{board[sq]};
            if (occ.is_piece()) {
                if (occ.piece_color() != friendly_color) {
                    moves.push_back({
                        .start_square = piece.square,
                        .target_square = sq,
                        .capture = {{sq, occ}},
                    });
                }
                break;
            }

            moves.push_back({piece.square, {sq}});
        }
    }

    return moves;
}

std::vector<ChessMove> MoveGenerator::generate_pawn_moves(
    const ChessBoard& board,
    SqOccPair pawn) {
    std::vector<ChessMove> moves{};
    ChessColor pawn_color = pawn.occ.piece_color();
    ChessSquare start_square = pawn.square;
    const auto dir{pawn_color == ChessColor::White ? north : south};
    const auto offset{direction_offsets[dir]};

    const bool first_move{
        ((pawn_color == ChessColor::White && start_square.row() == 1) ||
         (pawn_color == ChessColor::Black && start_square.row() == 6))};

    const int num_moves{
        std::min((first_move ? 2 : 1),
                 static_cast<int>(num_squares_to_edge[start_square][dir]))};
    for (int n{0}; n < num_moves; ++n) {
        const ChessSquare sq{start_square.offset_with(offset * (n + 1))};
        const SquareOccupant occ{board[sq]};
        if (occ.is_piece())
            break;

        if (n == 1) {
            moves.push_back({
                .start_square = start_square,
                .target_square = sq,
                .en_passant_target = sq.offset_with(-offset),
            });
        } else {
            moves.push_back({start_square, sq});
        }
    }

    // Calculate capture moves
    const auto en_passant_target{board.en_passant_target()};
    for (const ChessSquare sq :
         pawn_control_squares(start_square, pawn_color)) {
        const SquareOccupant occ{board[sq]};
        if (occ.is_piece() && occ.piece_color() != pawn_color) {
            moves.push_back({
                .start_square = start_square,
                .target_square = sq,
                .capture = {{sq, occ}},
            });
        } else if (en_passant_target && en_passant_target == sq) {
            moves.push_back({
                .start_square = start_square,
                .target_square = sq,
                .capture = {{sq.offset_with(-offset), occ}},
            });
        }
    }

    return moves;
}

std::vector<ChessMove> MoveGenerator::generate_knight_moves(
    const ChessBoard& board,
    SqOccPair knight) {
    std::vector<ChessMove> moves{};
    for (const auto sq : knight_control_squares(knight.square)) {
        const SquareOccupant occ{board[sq]};
        if (occ.is_piece()) {
            if (occ.piece_color() != knight.occ.piece_color()) {
                moves.push_back({
                    .start_square = knight.square,
                    .target_square = sq,
                    .capture = {{sq, occ}},
                });
            }
        } else {
            moves.push_back({knight.square, sq});
        }
    }

    return moves;
}

std::vector<ChessMove> MoveGenerator::generate_king_moves(
    const ChessBoard& board,
    SqOccPair king) {
    std::vector<ChessMove> moves{};
    for (const ChessSquare sq : king_control_squares(king.square)) {
        const SquareOccupant occ{board[sq]};
        if (occ.is_piece()) {
            if (occ.piece_color() != king.occ.piece_color()) {
                moves.push_back({
                    .start_square = king.square,
                    .target_square = sq,
                    .capture = {{sq, occ}},
                });
            }
        } else {
            moves.push_back({king.square, sq});
        }
    }

    const auto castle_rights{board.castle_rights()};
    const auto castle_sides{
        board.active_color() == ChessColor::White
            ? std::array<CastleSide, 2>{WhiteKingSide, WhiteQueenSide}
            : std::array<CastleSide, 2>{BlackKingSide, BlackQueenSide}};

    for (const auto& side : castle_sides) {
        if (castle_rights.has(side)) {
            const auto castle_move = castle_rights.castle_move(side);

            size_t begin = std::min(castle_move.rook_start, king.square) + 1;
            size_t end = std::max(castle_move.rook_start, king.square);

            bool is_blocked = std::ranges::any_of(
                std::views::iota(begin, end),
                [&](ChessSquare sq) { return board[sq].is_piece(); });

            if (!is_blocked) {
                moves.push_back({
                    .start_square = king.square,
                    .target_square = castle_move.king_target,
                    .castle_move = {{castle_move.rook_start,
                                     castle_move.rook_target}},
                });
            }
        }
    }

    return moves;
}

MoveEvalData MoveGenerator::generate_evaluation_data(
    const ChessBoard& board,
    std::vector<SqOccPair> hostile_pieces) {
    MoveEvalData data{board.passive_color()};

    for (const auto& piece : hostile_pieces) {
        switch (piece.occ.type()) {
            case OccupantType::Pawn:
                generate_pawn_evaluation_data(board, data, piece);
                break;
            case OccupantType::Knight:
                generate_knight_evaluation_data(board, data, piece);
                break;
            case OccupantType::King:
                generate_king_evaluation_data(board, data, piece);
                break;
            case OccupantType::Bishop:
            case OccupantType::Rook:
            case OccupantType::Queen:
                generate_sliding_evaluation_data(board, data, piece);
                break;
            default:
                break;
        }
    }

    return data;
}

void MoveGenerator::generate_sliding_evaluation_data(const ChessBoard& board,
                                                     MoveEvalData& data,
                                                     SqOccPair piece) {
    for (const auto& ray : sliding_piece_rays(piece.square, piece.occ.type())) {
        std::vector<ChessSquare> between{piece.square};
        std::optional<SqOccPair> first_piece{};

        for (const auto sq : ray) {
            const auto occ{board[sq]};
            between.push_back(sq);

            if (occ.is_empty()) {
                if (!first_piece.has_value()) {
                    ++data.hostile_control_map[sq];
                }
                continue;
            }

            // First piece on the ray
            if (!first_piece.has_value()) {
                first_piece = {sq, occ};
                ++data.hostile_control_map[sq];

                if (occ.piece_color() != data.hostile_color) {
                    if (occ.type() == OccupantType::King) {
                        // Direct check
                        data.hostile_checks.push_back(
                            CheckData{piece, between});
                        break;  // Pin not possible
                    }
                } else {
                    break;  // Pin not possible
                }
                continue;
            }

            // Second piece on the ray
            if (first_piece.value().occ.piece_color() != data.hostile_color &&
                occ.piece_color() != data.hostile_color &&
                occ.type() == OccupantType::King) {
                // Pin found
                data.hostile_pins.insert_or_assign({first_piece.value()},
                                                   between);
            }

            break;  // Ray blocked after second piece
        }
    }
}

void MoveGenerator::generate_pawn_evaluation_data(const ChessBoard& board,
                                                  MoveEvalData& data,
                                                  SqOccPair pawn) {
    for (const auto sq :
         pawn_control_squares(pawn.square, data.hostile_color)) {
        ++data.hostile_control_map[sq];
        const auto occ{board[sq]};
        if (occ.is_piece() && occ.piece_color() != data.hostile_color &&
            occ.type() == OccupantType::King) {
            data.hostile_checks.push_back({pawn, {pawn.square}});
        }
    }
}

void MoveGenerator::generate_knight_evaluation_data(const ChessBoard& board,
                                                    MoveEvalData& data,
                                                    SqOccPair knight) {
    for (const auto sq : knight_control_squares(knight.square)) {
        ++data.hostile_control_map[sq];
        const auto occ{board[sq]};
        if (occ.is_piece() && occ.piece_color() != data.hostile_color &&
            occ.type() == OccupantType::King) {
            data.hostile_checks.push_back({knight, {knight.square}});
        }
    }
}

void MoveGenerator::generate_king_evaluation_data(const ChessBoard& board,
                                                  MoveEvalData& data,
                                                  SqOccPair king) {
    for (const auto sq : king_control_squares(king.square)) {
        ++data.hostile_control_map[sq];
    }
}
