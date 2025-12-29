#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>
#include "ChessSquare.h"

constexpr std::array<char, 8> files{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
constexpr std::array<char, 8> ranks{'1', '2', '3', '4', '5', '6', '7', '8'};

struct ChessMove;

enum CastleSide {
    WhiteKingSide = 1 << 0,
    WhiteQueenSide = 1 << 1,
    BlackKingSide = 1 << 2,
    BlackQueenSide = 1 << 3,
};

constexpr ChessSquare w_king_square{'e', '1'};
constexpr ChessSquare b_king_square{'e', '8'};

constexpr ChessSquare w_king_rook{'h', '1'};
constexpr ChessSquare w_queen_rook{'a', '1'};
constexpr ChessSquare b_king_rook{'h', '8'};
constexpr ChessSquare b_queen_rook{'a', '8'};

constexpr std::array<CastleMove, 4> castle_moves{{{w_king_rook, {'f', '1'}, {'g', '1'}},
                                                  {w_queen_rook, {'d', '1'}, {'c', '1'}},
                                                  {b_king_rook, {'f', '8'}, {'g', '8'}},
                                                  {b_queen_rook, {'d', '8'}, {'c', '8'}}}};

struct CastleRights {
    uint8_t mask = 0;

    constexpr bool has(CastleSide side) const { return mask & side; }

    constexpr void remove(CastleSide side) { mask &= ~side; }
    constexpr void remove(uint8_t sides) { mask &= ~sides; }

    constexpr bool try_remove(ChessSquare mutated_square) {
        switch (mutated_square) {
            case w_king_rook: remove(WhiteKingSide); break;
            case w_queen_rook: remove(WhiteQueenSide); break;
            case b_king_rook: remove(BlackKingSide); break;
            case b_queen_rook: remove(BlackQueenSide); break;
            case w_king_square: remove(WhiteKingSide | WhiteQueenSide); break;
            case b_king_square: remove(BlackKingSide | BlackQueenSide); break;
            default: return false;
        }

        return true;
    }

    constexpr void add(CastleSide side) { mask |= side; }

    constexpr CastleMove castle_move(CastleSide side) const {
        switch (side) {
            case WhiteKingSide: return castle_moves[0];
            case WhiteQueenSide: return castle_moves[1];
            case BlackKingSide: return castle_moves[2];
            case BlackQueenSide: return castle_moves[3];
        }
    }

    static CastleRights from_fen(std::string_view fen = "KQkq") {
        CastleRights cr;
        for (char c : fen) {
            switch (c) {
                case 'K': cr.add(WhiteKingSide); break;
                case 'Q': cr.add(WhiteQueenSide); break;
                case 'k': cr.add(BlackKingSide); break;
                case 'q': cr.add(BlackQueenSide); break;
            }
        }
        return cr;
    }

    std::string as_fen() const {
        if (mask == 0)
            return "-";
        std::string s;
        if (has(WhiteKingSide))
            s += 'K';
        if (has(WhiteQueenSide))
            s += 'Q';
        if (has(BlackKingSide))
            s += 'k';
        if (has(BlackQueenSide))
            s += 'q';
        return s;
    }
};

/// FEN string: position, active color, castling rights, en passant targets
/// (optional), halfmove clock, fullmove number ref:
/// https://www.chess.com/terms/fen-chess
constexpr auto fen_start_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

class ChessBoard {
   public:
    ChessBoard(const std::string& fen_string = fen_start_position);
    ChessBoard(ChessBoard&&) = default;
    ChessBoard(const ChessBoard&) = default;
    ChessBoard& operator=(ChessBoard&&) = default;
    ChessBoard& operator=(const ChessBoard&) = default;
    ~ChessBoard() = default;

    const SquareOccupant& operator[](ChessSquare square) const;
    friend std::ostream& operator<<(std::ostream& os, const ChessBoard& board);

    void make_move(const ChessMove& move);
    void unmake_move(const ChessMove& move);
    void move_and_save(const ChessMove& move);
    ChessBoard copy_and_move(const ChessMove& move) const;
    ChessColor active_color() const { return m_active_color; }
    ChessColor passive_color() const {
        return m_active_color == ChessColor::White ? ChessColor::Black : ChessColor::White;
    }
    CastleRights castle_rights() const { return m_castle_rights; }
    std::optional<ChessSquare> en_passant_target() const { return m_en_passant_target; }
    const std::vector<std::string>& history() const { return m_history; }

    std::string as_fen() const;
    std::vector<SqOccPair> get_pieces(ChessColor piece_color) const;

   private:
    std::array<SquareOccupant, 64> m_position;
    size_t m_half_move_clock;
    size_t m_full_move_count;
    ChessColor m_active_color;
    CastleRights m_castle_rights;
    std::optional<ChessSquare> m_en_passant_target;
    std::vector<std::string> m_history;
};
