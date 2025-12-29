#include <immintrin.h>
#include "bitboard.h"

namespace Bitboards {

Bitboard RookTable[0x19000];   // To store rook attacks
Bitboard BishopTable[0x1480];  // To store bishop attacks

Magic Magics[SQUARE_NB][2];

uint8_t SquareDistance[SQUARE_NB][SQUARE_NB];

Bitboard BetweenBB[SQUARE_NB][SQUARE_NB];
Bitboard LineBB[SQUARE_NB][SQUARE_NB];

// Returns an ASCII representation of a bitboard suitable
// to be printed to standard output. Useful for debugging.
std::string pretty(Bitboard b) {
    std::string s = "+---+---+---+---+---+---+---+---+\n";

    for (Rank r = RANK_8; r >= RANK_1; --r) {
        for (File f = FILE_A; f <= FILE_H; ++f)
            s += b & make_square(f, r) ? "| 1 " : "| 0 ";

        s += "| " + std::to_string(1 + r) + "\n+---+---+---+---+---+---+---+---+\n";
    }
    s += "  a   b   c   d   e   f   g   h\n";

    return s;
}

// Todo: make this constexpr if possible

// Initializes various bitboard tables. It is called at
// startup and relies on global objects to be already zero-initialized.
void init() {
    for (Square s1 = SQ_A1; s1 <= SQ_H8; ++s1)
        for (Square s2 = SQ_A1; s2 <= SQ_H8; ++s2)
            SquareDistance[s1][s2] = std::max(distance<File>(s1, s2), distance<Rank>(s1, s2));

    init_magics(ROOK, RookTable, Magics);
    init_magics(BISHOP, BishopTable, Magics);

    for (Square s1 = SQ_A1; s1 <= SQ_H8; ++s1) {
        for (PieceType pt : {BISHOP, ROOK})
            for (Square s2 = SQ_A1; s2 <= SQ_H8; ++s2) {
                if (PseudoAttacks[pt][s1] & s2) {
                    LineBB[s1][s2] = (attacks_bb(pt, s1, 0) & attacks_bb(pt, s2, 0)) | s1 | s2;
                    BetweenBB[s1][s2] =
                        (attacks_bb(pt, s1, square_bb(s2)) & attacks_bb(pt, s2, square_bb(s1)));
                }
                BetweenBB[s1][s2] |= s2;
            }
    }
}

void init_magics(PieceType pt, Bitboard table[], Magic magics[][2]) {
    int size{0};
    for (Square s{SQ_A1}; s <= SQ_H8; ++s) {
        Bitboard edges{((Rank1BB | Rank8BB) & ~rank_bb(s)) | ((FileABB | FileHBB) & ~file_bb(s))};

        Magic& m{magics[s][pt - BISHOP]};
        m.mask = sliding_attack(pt, s, 0) & ~edges;

        m.attacks = s == SQ_A1 ? table : magics[s - 1][pt - BISHOP].attacks + size;
        size = 0;

        Bitboard b{0};
        do {
            m.attacks[_pext_u64(b, m.mask)] = sliding_attack(pt, s, b);

            ++size;
            b = (b - m.mask) & m.mask;
        } while (b);
    }
}

}  // namespace Bitboards
