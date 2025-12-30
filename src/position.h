#pragma once

#include <array>
#include <ostream>
#include "bitboard.h"

struct StateInfo {
    Square epSquare;
    CastlingRights castlingRights;
    int rule50;
};

/// FEN string: position, active color, castling rights, en passant targets
/// (optional), halfmove clock, fullmove number ref:
/// https://www.chess.com/terms/fen-chess
constexpr auto fen_start_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

class Position {
   public:
    Position(std::string fenStr = fen_start_position);
    std::string as_fen() const;
    // friend std::string pretty(const Position& p);
    // friend std::ostream& operator<<(std::ostream& os, const Position& p);

    // All pieces
    Bitboard pieces() const;
    Bitboard pieces(Color c) const;
    template <typename... PieceType>
    Bitboard pieces(PieceType... pts) const;
    template <typename... PieceType>
    Bitboard pieces(Color c, PieceType... pts) const;
    Piece piece_on(Square s) const;

    bool empty(Square s) const;
    bool can_castle(CastlingRights cr) const;

   private:
    std::array<Piece, SQUARE_NB> board{};
    std::array<Bitboard, COLOR_NB> byColorBB{};
    std::array<Bitboard, PIECE_TYPE_NB> byTypeBB{};
    StateInfo st;
    Color sideToMove;
    int gamePly;

    void put_piece(Piece p, Square s);
    void remove_piece(Square s);
    void move_piece(Square from, Square to);
    void set_castling_rights();
};

Position from_fen(const std::string fenStr);
