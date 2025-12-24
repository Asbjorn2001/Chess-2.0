#include <array>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>
#include "ChessSquare.h"

constexpr std::array<char, 8> files { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };
constexpr std::array<char, 8> ranks { '1', '2', '3', '4', '5', '6', '7', '8' };

struct ChessMove;

struct CastleRights {
    bool w_k_side;
    bool w_q_side;
    bool b_k_side;
    bool b_q_side;

    CastleRights(std::string fen_castle_str = "KQkq");
    std::string as_fen_string() const;
};

/// FEN string: position, active color, castling rights, en passant targets (optional), halfmove clock, fullmove number 
/// ref: https://www.chess.com/terms/fen-chess
constexpr auto fen_start_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq 0 1";

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

    void move(ChessMove move);
    ChessColor active_color() const { return m_active_color; } 
    CastleRights castle_rights() const { return m_castle_rights; }
    std::optional<ChessSquare> en_pessant_target() const { return m_en_passant_target; }
    const std::vector<std::string>& history() const { return m_history; }

    std::string as_fen_string() const;
    std::vector<std::pair<ChessSquare, SquareOccupant>> get_pieces(ChessColor piece_color) const;

private:
    std::array<SquareOccupant, 64> m_position;
    size_t m_half_move_clock;
    size_t m_full_move_count;
    ChessColor m_active_color;
    CastleRights m_castle_rights;
    std::optional<ChessSquare> m_en_passant_target;
    std::vector<std::string> m_history;
};
