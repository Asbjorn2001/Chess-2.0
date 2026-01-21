#pragma once

#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>

using Bitboard = uint64_t;
using Key = uint_fast64_t;

enum Color : int8_t {
    WHITE,
    BLACK,
    COLOR_NB = 2
};

enum CastlingRights : int8_t {
    NO_CASTLING,
    WHITE_OO,
    WHITE_OOO = WHITE_OO << 1,
    BLACK_OO = WHITE_OO << 2,
    BLACK_OOO = WHITE_OO << 3,

    KING_SIDE = WHITE_OO | BLACK_OO,
    QUEEN_SIDE = WHITE_OOO | BLACK_OOO,
    WHITE_CASTLING = WHITE_OO | WHITE_OOO,
    BLACK_CASTLING = BLACK_OO | BLACK_OOO,
    ANY_CASTLING = WHITE_CASTLING | BLACK_CASTLING,

    CASTLING_RIGHT_NB = 16
};

// clang-format off
enum PieceType : int8_t {
    NO_PIECE_TYPE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
    ALL_PIECES = 0,
    PIECE_TYPE_NB = 8
};

enum Piece : int8_t {
    NO_PIECE,
    W_PAWN = PAWN,     W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN = PAWN + 8, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
    PIECE_NB = 16
};
// clang-format on

// clang-format off
enum Square : int8_t {
    SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
    SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
    SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
    SQ_NONE,

    SQUARE_ZERO = 0,
    SQUARE_NB   = 64
};
// clang-format on

enum Direction : int8_t {
    NORTH = 8,
    EAST = 1,
    SOUTH = -NORTH,
    WEST = -EAST,

    NORTH_EAST = NORTH + EAST,
    SOUTH_EAST = SOUTH + EAST,
    SOUTH_WEST = SOUTH + WEST,
    NORTH_WEST = NORTH + WEST
};

enum File : int8_t {
    FILE_A,
    FILE_B,
    FILE_C,
    FILE_D,
    FILE_E,
    FILE_F,
    FILE_G,
    FILE_H,
    FILE_NB
};

enum Rank : int8_t {
    RANK_1,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8,
    RANK_NB
};

#define ENABLE_INCR_OPERATORS_ON(T) \
    constexpr T& operator++(T& d) { \
        return d = T(int(d) + 1);   \
    }                               \
    constexpr T& operator--(T& d) { \
        return d = T(int(d) - 1);   \
    }

ENABLE_INCR_OPERATORS_ON(PieceType)
ENABLE_INCR_OPERATORS_ON(Square)
ENABLE_INCR_OPERATORS_ON(File)
ENABLE_INCR_OPERATORS_ON(Rank)

#undef ENABLE_INCR_OPERATORS_ON

constexpr Direction operator+(Direction d1, Direction d2) {
    return Direction(int(d1) + int(d2));
}
constexpr Direction operator*(int i, Direction d) {
    return Direction(i * int(d));
}

// Additional operators to add a Direction to a Square
constexpr Square operator+(Square s, Direction d) {
    return Square(int(s) + int(d));
}
constexpr Square operator-(Square s, Direction d) {
    return Square(int(s) - int(d));
}
constexpr Square& operator+=(Square& s, Direction d) {
    return s = s + d;
}
constexpr Square& operator-=(Square& s, Direction d) {
    return s = s - d;
}

// Toggle color
constexpr Color operator~(Color c) {
    return Color(c ^ BLACK);
}

// Swap A1 <-> A8
constexpr Square flip_rank(Square s) {
    return Square(s ^ SQ_A8);
}

// Swap A1 <-> H1
constexpr Square flip_file(Square s) {
    return Square(s ^ SQ_H1);
}

// Swap color of piece B_KNIGHT <-> W_KNIGHT
constexpr Piece operator~(Piece pc) {
    return Piece(pc ^ 8);
}

constexpr CastlingRights operator&(Color c, CastlingRights cr) {
    return CastlingRights((c == WHITE ? WHITE_CASTLING : BLACK_CASTLING) & cr);
}

constexpr Square make_square(File f, Rank r) {
    return Square((r << 3) + f);
}

constexpr Piece make_piece(Color c, PieceType pt) {
    return Piece((c << 3) + pt);
}

constexpr PieceType type_of(Piece pc) {
    return PieceType(pc & 7);
}

constexpr Color color_of(Piece pc) {
    assert(pc != NO_PIECE);
    return Color(pc >> 3);
}

constexpr bool is_ok(Square s) {
    return s >= SQ_A1 && s <= SQ_H8;
}

constexpr File file_of(Square s) {
    return File(s & 7);
}

constexpr Rank rank_of(Square s) {
    return Rank(s >> 3);
}

constexpr Square relative_square(Color c, Square s) {
    return Square(s ^ (c * 56));
}

constexpr Rank relative_rank(Color c, Rank r) {
    return Rank(r ^ (c * 7));
}

constexpr Rank relative_rank(Color c, Square s) {
    return relative_rank(c, rank_of(s));
}

constexpr Direction pawn_push(Color c) {
    return c == WHITE ? NORTH : SOUTH;
}

constexpr CastlingRights cr_from_color(Color c) {
    return c == WHITE ? WHITE_CASTLING : BLACK_CASTLING;
}

constexpr CastlingRights cr_from_sq(Square s) {
    switch (s) {
        case SQ_A1: return WHITE_OOO;
        case SQ_H1: return WHITE_OO;
        case SQ_A8: return BLACK_OOO;
        case SQ_H8: return BLACK_OO;
        case SQ_E1: return WHITE_CASTLING;
        case SQ_E8: return BLACK_CASTLING;
        default: return NO_CASTLING;
    }
}

constexpr std::array<char, 12> PieceChars = {'P', 'N', 'B', 'R', 'Q', 'K',
                                             'p', 'n', 'b', 'r', 'q', 'k'};

template <std::integral T>
constexpr inline Piece pc_from_index(T i) {
    assert(i >= 0 && i < 12);  // There are only 12 pieces
    return i < 6 ? Piece(i + 1) : Piece(i + 3);
}

constexpr inline size_t pc_as_index(Piece p) {
    return color_of(p) == WHITE ? p - 1 : p - 3;
}

constexpr inline char pc_as_char(Piece p) {
    return PieceChars[pc_as_index(p)];
}

constexpr Piece pc_from_char(char c) {
    switch (c) {
        case 'P': return W_PAWN;
        case 'N': return W_KNIGHT;
        case 'B': return W_BISHOP;
        case 'R': return W_ROOK;
        case 'Q': return W_QUEEN;
        case 'K': return W_KING;
        case 'p': return B_PAWN;
        case 'n': return B_KNIGHT;
        case 'b': return B_BISHOP;
        case 'r': return B_ROOK;
        case 'q': return B_QUEEN;
        case 'k': return B_KING;
        default: return NO_PIECE;
    }
}

enum MoveType {
    NORMAL,
    PROMOTION = 1 << 14,
    EN_PASSANT = 2 << 14,
    CASTLING = 3 << 14
};

#define ENABLE_BITWISE_OPERATORS_ON(T)       \
    constexpr T operator|(T lhs, T rhs) {    \
        return T(int(lhs) | int(rhs));       \
    }                                        \
    constexpr T& operator|=(T& lhs, T rhs) { \
        return lhs = T(int(lhs) | int(rhs)); \
    }                                        \
    constexpr T operator&(T lhs, T rhs) {    \
        return T(int(lhs) & int(rhs));       \
    }                                        \
    constexpr T& operator&=(T& lhs, T rhs) { \
        return lhs = T(int(lhs) & int(rhs)); \
    }

ENABLE_BITWISE_OPERATORS_ON(CastlingRights)

#undef ENABLE_BITWISE_OPERATORS_ON

constexpr CastlingRights& operator~(CastlingRights& cr) {
    return cr = ANY_CASTLING & CastlingRights(~int(cr));
}

// A move needs 16 bits to be stored
//
// bit  0- 5: destination square (from 0 to 63)
// bit  6-11: origin square (from 0 to 63)
// bit 12-13: promotion piece type - 2 (from KNIGHT-2 to QUEEN-2)
// bit 14-15: special move flag: promotion (1), en passant (2), castling (3)
// NOTE: en passant bit is set only when a pawn can be captured
//
// Special cases are Move::none() and Move::null(). We can sneak these in
// because in any normal move the destination square and origin square are
// always different, but Move::none() and Move::null() have the same origin and
// destination square.

class Move {
   public:
    Move() = default;
    Move(uint16_t data) : data(data) {}

    Move(Square from, Square to) : data(static_cast<uint16_t>((from << 6) | to)) {}

    Move(Square from, Square to, uint16_t flags)
        : data(static_cast<uint16_t>((flags << 12) | ((from & 0x3f) << 6) | to)) {}

    template <MoveType T>
    static constexpr Move make(Square from, Square to, PieceType pt = KNIGHT) {
        return Move(T + ((pt - KNIGHT) << 12) + (from << 6) + to);
    }

    constexpr Square from_sq() const {
        assert(is_ok());
        return Square((data >> 6) & 0x3F);
    }

    constexpr Square to_sq() const {
        assert(is_ok());
        return Square(data & 0x3F);
    }

    constexpr MoveType type_of() const { return MoveType(data & (3 << 14)); }

    constexpr PieceType promotion_type() const { return PieceType(((data >> 12) & 3) + KNIGHT); }

    constexpr bool is_ok() const { return none().data != data && null().data != data; }

    static constexpr Move null() { return Move(65); }
    static constexpr Move none() { return Move(0); }

    constexpr bool operator==(const Move& rhs) const { return data == rhs.data; }
    constexpr bool operator!=(const Move& rhs) const { return data != rhs.data; }

    constexpr explicit operator bool() const { return data != 0; }

    constexpr std::uint16_t raw() const { return data; }

   protected:
    uint16_t data;
};
