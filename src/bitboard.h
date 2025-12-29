#include <immintrin.h>
#include <popcntintrin.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <ostream>
#include "types.h"

namespace Bitboards {

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

extern uint8_t SquareDistance[SQUARE_NB][SQUARE_NB];

extern Bitboard BetweenBB[SQUARE_NB][SQUARE_NB];
extern Bitboard LineBB[SQUARE_NB][SQUARE_NB];

std::string pretty(Bitboard b);

constexpr Bitboard square_bb(Square s) {
    assert(is_ok(s));
    return (1ULL << s);
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
    return b & s;
}
constexpr Bitboard operator|(Square s, Bitboard b) {
    return b | s;
}
constexpr Bitboard operator^(Square s, Bitboard b) {
    return b ^ s;
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

template <typename T1 = Square>
inline int distance(Square x, Square y);

template <>
inline int distance<File>(Square x, Square y) {
    return std::abs(file_of(x) - file_of(y));
}

template <>
inline int distance<Rank>(Square x, Square y) {
    return std::abs(rank_of(x) - rank_of(y));
}

template <>
inline int distance<Square>(Square x, Square y) {
    return SquareDistance[x][y];
}

inline int edge_distance(File f) {
    return std::min(f, File(FILE_H - f));
}

// constexpr inline Bitboard pext(Bitboard b, Bitboard m) {
//     return _pext_u64(b, m);
// }

// inline int64_t popcount(Bitboard b) {
//     return _mm_popcnt_u64(b);
// }

struct Magic {
    Bitboard mask;
    Bitboard* attacks;

    inline unsigned long long index(Bitboard occupied) const {
        return _pext_u64(static_cast<unsigned long long>(occupied),
                         static_cast<unsigned long long>(mask));
    }

    Bitboard attacks_bb(Bitboard occupied) const { return attacks[index(occupied)]; }
};

extern Magic Magics[SQUARE_NB][2];

template <Direction D>
constexpr Bitboard shift(Bitboard b) {
    switch (D) {
        case NORTH:
            return b << 8;
        case SOUTH:
            return b >> 8;
        case EAST:
            return (b & ~FileHBB) << 1;
        case WEST:
            return (b & ~FileABB) >> 1;
        case NORTH_EAST:
            return (b & ~FileHBB) << 9;
        case NORTH_WEST:
            return (b & ~FileABB) << 7;
        case SOUTH_EAST:
            return (b & ~FileHBB) >> 7;
        case SOUTH_WEST:
            return (b & ~FileABB) >> 9;
    }
}

template <Color C>
constexpr Bitboard pawn_attacks_bb(Bitboard b) {
    return C == WHITE ? shift<NORTH_WEST>(b) | shift<NORTH_EAST>(b)
                      : shift<SOUTH_WEST>(b) | shift<SOUTH_EAST>(b);
}

// Returns the bitboard of target square for the given step
// from the given square. If the step is off the board, returns empty bitboard.
constexpr Bitboard safe_destination(Square s, int step) {
    constexpr auto abs = [](int v) { return v < 0 ? -v : v; };
    Square to = Square(s + step);
    return is_ok(to) && abs(file_of(s) - file_of(to)) <= 2 ? square_bb(to) : Bitboard(0);
}

constexpr Bitboard sliding_attack(PieceType pt, Square sq, Bitboard occupied) {
    Bitboard attacks{0};
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
    Bitboard b = {};
    for (int step : {-17, -15, -10, -6, 6, 10, 15, 17})
        b |= safe_destination(sq, step);
    return b;
}

constexpr Bitboard king_attack(Square sq) {
    Bitboard b = {};
    for (int step : {-9, -8, -7, -1, 1, 7, 8, 9})
        b |= safe_destination(sq, step);
    return b;
}

constexpr Bitboard pseudo_attack(PieceType pt, Square sq) {
    switch (pt) {
        case PieceType::ROOK:
        case PieceType::BISHOP:
            return sliding_attack(pt, sq, 0);
        case PieceType::QUEEN:
            return sliding_attack(PieceType::ROOK, sq, 0) |
                   sliding_attack(PieceType::BISHOP, sq, 0);
        case PieceType::KNIGHT:
            return knight_attack(sq);
        case PieceType::KING:
            return king_attack(sq);
        default:
            assert(false);
            return 0;
    }
}

inline constexpr auto PseudoAttacks = []() constexpr {
    std::array<std::array<Bitboard, SQUARE_NB>, PIECE_TYPE_NB> attacks{};

    for (Square s{SQ_A1}; s <= SQ_H8; ++s) {
        attacks[WHITE][s] = pawn_attacks_bb<WHITE>(square_bb(s));
        attacks[BLACK][s] = pawn_attacks_bb<BLACK>(square_bb(s));

        attacks[KNIGHT][s] = pseudo_attack(KNIGHT, s);
        attacks[BISHOP][s] = pseudo_attack(BISHOP, s);
        attacks[ROOK][s] = pseudo_attack(ROOK, s);
        attacks[QUEEN][s] = pseudo_attack(QUEEN, s);
        attacks[KING][s] = pseudo_attack(KING, s);
    }

    return attacks;
}();

// Returns the pseudo attacks of the given piece type
// assuming an empty board.
template <PieceType Pt>
inline Bitboard attacks_bb(Square s, Color c = COLOR_NB) {
    assert((Pt != PAWN || c < COLOR_NB) && (is_ok(s)));
    return Pt == PAWN ? PseudoAttacks[c][s] : PseudoAttacks[Pt][s];
}

// Returns the attacks by the given piece
// assuming the board is occupied according to the passed Bitboard.
// Sliding piece attacks do not continue passed an occupied square.
template <PieceType Pt>
inline Bitboard attacks_bb(Square s, Bitboard occupied) {
    assert((Pt != PAWN) && (is_ok(s)));

    switch (Pt) {
        case BISHOP:
        case ROOK:
            return Magics[s][Pt - BISHOP].attacks_bb(occupied);
        case QUEEN:
            return attacks_bb<BISHOP>(s, occupied) | attacks_bb<ROOK>(s, occupied);
        default:
            return PseudoAttacks[Pt][s];
    }
}

// Returns the attacks by the given piece
// assuming the board is occupied according to the passed Bitboard.
// Sliding piece attacks do not continue passed an occupied square.
inline Bitboard attacks_bb(PieceType pt, Square s, Bitboard occupied) {
    assert((pt != PAWN) && (is_ok(s)));

    switch (pt) {
        case BISHOP:
            return attacks_bb<BISHOP>(s, occupied);
        case ROOK:
            return attacks_bb<ROOK>(s, occupied);
        case QUEEN:
            return attacks_bb<BISHOP>(s, occupied) | attacks_bb<ROOK>(s, occupied);
        default:
            return PseudoAttacks[pt][s];
    }
}

inline Bitboard attacks_bb(Piece pc, Square s) {
    if (type_of(pc) == PAWN)
        return PseudoAttacks[color_of(pc)][s];

    return PseudoAttacks[type_of(pc)][s];
}

inline Bitboard attacks_bb(Piece pc, Square s, Bitboard occupied) {
    if (type_of(pc) == PAWN)
        return PseudoAttacks[color_of(pc)][s];

    return attacks_bb(type_of(pc), s, occupied);
}

void init();
void init_magics(PieceType pt, Bitboard table[], Magic magics[][2]);

}  // namespace Bitboards
