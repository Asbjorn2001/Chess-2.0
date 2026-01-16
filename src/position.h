#pragma once

#include <array>
#include <string>
#include "bitboard.h"
#include "types.h"

constexpr auto CastlingPaths = []() {
    std::array<Bitboard, CASTLING_RIGHT_NB> paths{};
    paths[WHITE_OO] = SQ_F1 | SQ_G1;
    paths[WHITE_OOO] = SQ_B1 | SQ_C1 | SQ_D1;
    paths[BLACK_OO] = SQ_F8 | SQ_G8;
    paths[BLACK_OOO] = SQ_B8 | SQ_C8 | SQ_D8;

    return paths;
}();

constexpr auto CastlingRookSquares = []() {
    std::array<Square, CASTLING_RIGHT_NB> squares{};
    squares[WHITE_OO] = SQ_H1;
    squares[WHITE_OOO] = SQ_A1;
    squares[BLACK_OO] = SQ_H8;
    squares[BLACK_OOO] = SQ_A8;

    return squares;
}();

constexpr Bitboard RookSquares = SQ_A1 | SQ_A8 | SQ_H1 | SQ_H8;
constexpr Bitboard KingSquares = SQ_E1 | SQ_E8;
constexpr Bitboard CastlingSquares = KingSquares | RookSquares;

struct StateInfo {
    StateInfo() = default;

    Square epSquare = SQ_NONE;
    CastlingRights castlingRights;
    int rule50;
    Bitboard checkersBB{};
    Bitboard blockersForKing[COLOR_NB];
    Bitboard pinners[COLOR_NB];
    Piece capturedPiece = NO_PIECE;
    StateInfo* previous;
};

/// FEN string: position, active color, castling rights, en passant targets
/// (optional), halfmove clock, fullmove number ref:
/// https://www.chess.com/terms/fen-chess
constexpr auto fen_start_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

class Position {
   public:
    Position(std::string fenStr = fen_start_position);
    ~Position() { delete st; }
    Position(const Position& rhs) : st(new StateInfo(*rhs.st)) {}
    Position& operator=(const Position& rhs) {
        if (this != &rhs) {
            st = new StateInfo{*rhs.st};
        }
        return *this;
    }
    Position(Position&& rhs) : st(rhs.st) { rhs.st = nullptr; }
    Position& operator=(Position&& rhs) {
        if (this != &rhs) {
            delete st;
            st = rhs.st;
            rhs.st = nullptr;
        }
        return *this;
    }

    std::string as_fen() const;

    // All pieces
    Bitboard pieces() const;
    Bitboard pieces(Color c) const;
    template <PieceType... Pts>
    Bitboard pieces() const;
    template <PieceType... Pts>
    Bitboard pieces(Color c) const;

    Piece piece_on(Square s) const;
    template <PieceType Pt>
    Square square(Color c) const;

    bool legal(Move m) const;
    bool pseudo_legal(Move m) const;

    void make_move(Move m);
    void unmake_move(Move m);
    Piece moved_piece(Move m) const;

    bool is_empty(Square s) const;

    bool can_castle(CastlingRights cr) const;
    bool castling_impeded(CastlingRights cr) const;
    Square castling_rook_square(CastlingRights cr) const;

    // Public getters
    Color side_to_move() const;
    Bitboard checkers() const;
    Bitboard blockers_for_king(Color c) const;
    Square ep_square() const;
    std::array<Piece, SQUARE_NB> board() const;
    const StateInfo* state() const;

   private:
    std::array<Piece, SQUARE_NB> board_{};
    std::array<Bitboard, COLOR_NB> byColorBB{};
    std::array<Bitboard, PIECE_TYPE_NB> byTypeBB{};
    StateInfo* st{};
    Color sideToMove;
    int gamePly;

    void put_piece(Piece p, Square s);
    void remove_piece(Square s);
    void move_piece(Square from, Square to);
    void update_slider_blockers(Color c);
    void update_state_info(StateInfo* newState);

    void set_castling_rights(CastlingRights cr);
    void remove_castling_rights(CastlingRights cr);

    Bitboard attackers_to(Square s) const;
    template <PieceType... Pts>
    Bitboard attackers_to(Square s) const;

    Bitboard attackers_to(Square s, Bitboard occupied) const;
    template <PieceType... Pts>
    Bitboard attackers_to(Square s, Bitboard occupied) const;

    bool attackers_to_exist(Square s, Bitboard occupied, Color c) const;
    template <PieceType... Pts>
    bool attackers_to_exist(Square s, Bitboard occupied, Color c) const;
};

inline Color Position::side_to_move() const {
    return sideToMove;
}

inline Bitboard Position::blockers_for_king(Color c) const {
    return st->blockersForKing[c];
}

inline Bitboard Position::checkers() const {
    return st->checkersBB;
}

inline Square Position::ep_square() const {
    return st->epSquare;
}

inline std::array<Piece, SQUARE_NB> Position::board() const {
    return board_;
}

template <PieceType Pt>
inline Square Position::square(Color c) const {
    return lsb(pieces<Pt>(c));
}

inline Piece Position::moved_piece(Move m) const {
    return piece_on(m.from_sq());
}

inline Bitboard Position::pieces() const {
    return byTypeBB[ALL_PIECES];
}

inline Bitboard Position::pieces(Color c) const {
    return byColorBB[c];
}

template <PieceType... Pts>
inline Bitboard Position::pieces() const {
    return (byTypeBB[Pts] | ...);
}

template <PieceType... Pts>
inline Bitboard Position::pieces(Color c) const {
    return pieces(c) & pieces<Pts...>();
}

// Computes a bitboard of all pieces which attack a given square.
// Slider attacks use the occupied bitboard to indicate occupancy.
inline Bitboard Position::attackers_to(Square s) const {
    return attackers_to(s, pieces());
}

inline Bitboard Position::attackers_to(Square s, Bitboard occupied) const {
    return attackers_to<KNIGHT, BISHOP, ROOK, QUEEN, KING>(s, occupied) |
           (attacks_bb<PAWN>(s, BLACK) & pieces<PAWN>(WHITE)) |
           (attacks_bb<PAWN>(s, WHITE) & pieces<PAWN>(BLACK));
}

template <PieceType... Pts>
inline Bitboard Position::attackers_to(Square s) const {
    return attackers_to<Pts...>(s, pieces());
}

template <PieceType... Pts>
inline Bitboard Position::attackers_to(Square s, Bitboard occupied) const {
    return ((attacks_bb(Pts, s, occupied) & pieces<Pts>()) | ...);
}

inline bool Position::attackers_to_exist(Square s, Bitboard occupied, Color c) const {
    return attackers_to(s, occupied) & pieces(c);
}

template <PieceType... Pts>
inline bool Position::attackers_to_exist(Square s, Bitboard occupied, Color c) const {
    return attackers_to<Pts...>(s, occupied) & pieces(c);
}

inline void Position::set_castling_rights(CastlingRights cr) {
    st->castlingRights |= cr;
}

inline void Position::remove_castling_rights(CastlingRights cr) {
    st->castlingRights &= ~cr;
}

inline const StateInfo* Position::state() const {
    return st;
}
