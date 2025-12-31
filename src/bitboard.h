#pragma once

#include <immintrin.h>
#include <popcntintrin.h>
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include "types.h"

namespace Bitboards {
void init();
}

constexpr Bitboard FileABB = 0x0101010101010101ULL;
constexpr Bitboard FileBBB = FileABB << 1;
constexpr Bitboard FileCBB = FileABB << 2;
constexpr Bitboard FileDBB = FileABB << 3;
constexpr Bitboard FileEBB = FileABB << 4;
constexpr Bitboard FileFBB = FileABB << 5;
constexpr Bitboard FileGBB = FileABB << 6;
constexpr Bitboard FileHBB = FileABB << 7;

constexpr Bitboard Rank1BB = 0xFF;
constexpr Bitboard Rank2BB = Rank1BB << (8 * 1);
constexpr Bitboard Rank3BB = Rank1BB << (8 * 2);
constexpr Bitboard Rank4BB = Rank1BB << (8 * 3);
constexpr Bitboard Rank5BB = Rank1BB << (8 * 4);
constexpr Bitboard Rank6BB = Rank1BB << (8 * 5);
constexpr Bitboard Rank7BB = Rank1BB << (8 * 6);
constexpr Bitboard Rank8BB = Rank1BB << (8 * 7);

struct Magic;
extern Magic Magics[SQUARE_NB][2];

extern Bitboard BetweenBB[SQUARE_NB][SQUARE_NB];
extern Bitboard LineBB[SQUARE_NB][SQUARE_NB];

constexpr Bitboard square_bb(Square s) {
    assert(is_ok(s));
    return (1ULL << s);
}

// Returns a bitboard representing the squares in the semi-open
// segment between the squares s1 and s2 (excluding s1 but including s2). If the
// given squares are not on a same file/rank/diagonal, it returns s2. For instance,
// between_bb(SQ_C4, SQ_F7) will return a bitboard with squares D5, E6 and F7, but
// between_bb(SQ_E6, SQ_F8) will return a bitboard with the square F8. This trick
// allows to generate non-king evasion moves faster: the defending piece must either
// interpose itself to cover the check or capture the checking piece.
constexpr Bitboard between_bb(Square s1, Square s2) {
    return BetweenBB[s1][s2];
}

// Returns a bitboard representing an entire line (from board edge
// to board edge) that intersects the two given squares. If the given squares
// are not on a same file/rank/diagonal, the function returns 0. For instance,
// line_bb(SQ_C4, SQ_F7) will return a bitboard with the A2-G8 diagonal.
constexpr Bitboard line_bb(Square s1, Square s2) {
    return LineBB[s1][s2];
}

// Overloads of bitwise operators between a Bitboard and a Square for testing
// whether a given bit is set in a bitboard, and for setting and clearing bits.

constexpr Bitboard operator&(Bitboard b, Square s) {
    return b & square_bb(s);
}
constexpr Bitboard operator|(Bitboard b, Square s) {
    return b | square_bb(s);
}
constexpr Bitboard operator^(Bitboard b, Square s) {
    return b ^ square_bb(s);
}
constexpr Bitboard& operator|=(Bitboard& b, Square s) {
    return b |= square_bb(s);
}
constexpr Bitboard& operator^=(Bitboard& b, Square s) {
    return b ^= square_bb(s);
}

constexpr Bitboard operator&(Square s, Bitboard b) {
    return b & square_bb(s);
}
constexpr Bitboard operator|(Square s, Bitboard b) {
    return b | square_bb(s);
}
constexpr Bitboard operator^(Square s, Bitboard b) {
    return b ^ square_bb(s);
}

constexpr Bitboard operator|(Square s1, Square s2) {
    return square_bb(s1) | s2;
}

constexpr bool more_than_one(Bitboard b) {
    return b & (b - 1);
}

// rank_bb() and file_bb() return a bitboard representing all the squares on
// the given file or rank.

constexpr Bitboard rank_bb(Rank r) {
    return Rank1BB << (8 * r);
}

constexpr Bitboard rank_bb(Square s) {
    return rank_bb(rank_of(s));
}

constexpr Bitboard file_bb(File f) {
    return FileABB << f;
}

constexpr Bitboard file_bb(Square s) {
    return file_bb(file_of(s));
}

constexpr int constexpr_abs(int num) {
    return num < 0 ? -num : num;
}

template <typename T1 = Square>
inline constexpr int distance(Square x, Square y);

template <>
inline constexpr int distance<File>(Square x, Square y) {
    return constexpr_abs(file_of(x) - file_of(y));
}

template <>
inline constexpr int distance<Rank>(Square x, Square y) {
    return constexpr_abs(rank_of(x) - rank_of(y));
}

inline constexpr int edge_distance(File f) {
    return std::min(f, File(FILE_H - f));
}

inline constexpr Bitboard pext(Bitboard b, Bitboard m) {
    return _pext_u64(b, m);
}

inline constexpr int64_t popcount(Bitboard b) {
    return _mm_popcnt_u64(b);
}

inline constexpr Square lsb(Bitboard b) {
    assert(b);
    return Square(__builtin_ctzll(b));
}

inline constexpr Square msb(Bitboard b) {
    assert(b);
    return Square(63 ^ __builtin_clzll(b));
}

// Finds and clears the least significant bit in a non-zero bitboard.
inline Square pop_lsb(Bitboard& b) {
    assert(b);
    const Square s = lsb(b);
    b &= b - 1;
    return s;
}

inline constexpr auto SquareDistance = []() constexpr {
    std::array<std::array<int8_t, SQUARE_NB>, SQUARE_NB> distances{};
    for (Square s1 = SQ_A1; s1 <= SQ_H8; ++s1) {
        for (Square s2 = SQ_A1; s2 <= SQ_H8; ++s2) {
            distances[s1][s2] = std::max(distance<File>(s1, s2), distance<Rank>(s1, s2));
        }
    }

    return distances;
}();

template <>
inline constexpr int distance<Square>(Square x, Square y) {
    return SquareDistance[x][y];
}

struct Magic {
    Bitboard mask;
    Bitboard* attacks;
    uint64_t size;

    inline constexpr uint64_t index(Bitboard occupied) const { return pext(occupied, mask); }

    constexpr Bitboard attacks_bb(Bitboard occupied) const {
        assert(index(occupied) < size);
        return attacks[index(occupied)];
    }
};

template <Direction D>
constexpr Bitboard shift(Bitboard b) {
    switch (D) {
        case NORTH: return b << 8;
        case SOUTH: return b >> 8;
        case EAST: return (b & ~FileHBB) << 1;
        case WEST: return (b & ~FileABB) >> 1;
        case NORTH_EAST: return (b & ~FileHBB) << 9;
        case NORTH_WEST: return (b & ~FileABB) << 7;
        case SOUTH_EAST: return (b & ~FileHBB) >> 7;
        case SOUTH_WEST: return (b & ~FileABB) >> 9;
    }
}

template <Color C>
constexpr Bitboard pawn_attacks_bb(Bitboard b) {
    return C == WHITE ? shift<NORTH_WEST>(b) | shift<NORTH_EAST>(b)
                      : shift<SOUTH_WEST>(b) | shift<SOUTH_EAST>(b);
}

namespace Bitboards {

// Returns the bitboard of target square for the given step
// from the given square. If the step is off the board, returns empty bitboard.
constexpr Bitboard safe_destination(Square s, int step) {
    constexpr auto abs = [](int v) { return v < 0 ? -v : v; };
    Square to = Square(s + step);
    return is_ok(to) && abs(file_of(s) - file_of(to)) <= 2 ? square_bb(to) : Bitboard(0);
}

constexpr Bitboard sliding_attack(PieceType pt, Square sq, Bitboard occupied) {
    assert(pt >= BISHOP && pt <= QUEEN && is_ok(sq));
    Bitboard attacks{};
    Direction rook_directions[4]{NORTH, SOUTH, EAST, WEST};
    Direction bishop_directions[4]{NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST};

    for (Direction d : (pt == ROOK ? rook_directions : bishop_directions)) {
        Square s{sq};
        while (safe_destination(s, d)) {
            attacks |= (s += d);
            if (s & occupied) {
                break;
            }
        }
    }
    return attacks;
}

constexpr Bitboard knight_attack(Square sq) {
    Bitboard b{};
    for (int step : {-17, -15, -10, -6, 6, 10, 15, 17})
        b |= safe_destination(sq, step);
    return b;
}

constexpr Bitboard king_attack(Square sq) {
    Bitboard b{};
    for (int step : {-9, -8, -7, -1, 1, 7, 8, 9})
        b |= safe_destination(sq, step);
    return b;
}

constexpr Bitboard pseudo_attack(PieceType pt, Square sq) {
    switch (pt) {
        case PieceType::ROOK:
        case PieceType::BISHOP: return sliding_attack(pt, sq, 0);
        case PieceType::QUEEN:
            return sliding_attack(PieceType::ROOK, sq, 0) |
                   sliding_attack(PieceType::BISHOP, sq, 0);
        case PieceType::KNIGHT: return knight_attack(sq);
        case PieceType::KING: return king_attack(sq);
        default: assert(false); return 0;
    }
}

}  // namespace Bitboards

inline constexpr auto PseudoAttacks = []() constexpr {
    std::array<std::array<Bitboard, SQUARE_NB>, PIECE_TYPE_NB> attacks{};

    for (Square s{SQ_A1}; s <= SQ_H8; ++s) {
        attacks[WHITE][s] = pawn_attacks_bb<WHITE>(square_bb(s));
        attacks[BLACK][s] = pawn_attacks_bb<BLACK>(square_bb(s));

        attacks[KNIGHT][s] = Bitboards::pseudo_attack(KNIGHT, s);
        attacks[BISHOP][s] = Bitboards::pseudo_attack(BISHOP, s);
        attacks[ROOK][s] = Bitboards::pseudo_attack(ROOK, s);
        attacks[QUEEN][s] = Bitboards::pseudo_attack(QUEEN, s);
        attacks[KING][s] = Bitboards::pseudo_attack(KING, s);
    }

    return attacks;
}();

// Returns the pseudo attacks of the given piece type
// assuming an empty board.
template <PieceType Pt>
inline constexpr Bitboard attacks_bb(Square s, Color c = COLOR_NB) {
    assert((Pt != PAWN || c < COLOR_NB) && is_ok(s));
    return Pt == PAWN ? PseudoAttacks[c][s] : PseudoAttacks[Pt][s];
}

// Returns the attacks by the given piece
// assuming the board is occupied according to the passed Bitboard.
// Sliding piece attacks do not continue passed an occupied square.
template <PieceType Pt>
inline constexpr Bitboard attacks_bb(Square s, Bitboard occupied) {
    assert((Pt != PAWN) && (is_ok(s)));

    switch (Pt) {
        case BISHOP:
        case ROOK: assert(Pt - BISHOP < 2); return Magics[s][Pt - BISHOP].attacks_bb(occupied);
        case QUEEN: return attacks_bb<BISHOP>(s, occupied) | attacks_bb<ROOK>(s, occupied);
        default: return PseudoAttacks[Pt][s];
    }
}

// Returns the attacks by the given piece
// assuming the board is occupied according to the passed Bitboard.
// Sliding piece attacks do not continue passed an occupied square.
inline constexpr Bitboard attacks_bb(PieceType pt, Square s, Bitboard occupied) {
    assert((pt != PAWN) && (is_ok(s)));

    switch (pt) {
        case BISHOP: return attacks_bb<BISHOP>(s, occupied);
        case ROOK: return attacks_bb<ROOK>(s, occupied);
        case QUEEN: return attacks_bb<BISHOP>(s, occupied) | attacks_bb<ROOK>(s, occupied);
        default: return PseudoAttacks[pt][s];
    }
}

inline constexpr Bitboard attacks_bb(Piece pc, Square s) {
    if (type_of(pc) == PAWN) {
        return PseudoAttacks[color_of(pc)][s];
    }

    return PseudoAttacks[type_of(pc)][s];
}

inline constexpr Bitboard attacks_bb(Piece pc, Square s, Bitboard occupied) {
    if (type_of(pc) == PAWN) {
        return PseudoAttacks[color_of(pc)][s];
    }

    return attacks_bb(type_of(pc), s, occupied);
}
