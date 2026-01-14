#include <immintrin.h>
#include "bitboard.h"

namespace Bitboards {                 // Make sure it is only initialized once
inline Bitboard RookTable[0x19000];   // To store rook attacks
inline Bitboard BishopTable[0x1480];  // To store bishop attacks

inline void init_magics(PieceType pt, Bitboard table[], Magic magics[][2]);
}  // namespace Bitboards

alignas(64) Magic Magics[SQUARE_NB][2];

Bitboard BetweenBB[SQUARE_NB][SQUARE_NB];
Bitboard LineBB[SQUARE_NB][SQUARE_NB];

// Initializes various bitboard tables. It is called at
// startup and relies on global objects to be already zero-initialized.
void Bitboards::init() {
    std::cout << "Initializing bitboards...\n";
    Bitboards::init_magics(ROOK, RookTable, Magics);
    Bitboards::init_magics(BISHOP, BishopTable, Magics);

    for (Square s1 = SQ_A1; s1 <= SQ_H8; ++s1) {
        for (PieceType pt : {BISHOP, ROOK}) {
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
}

namespace Bitboards {

void init_magics(PieceType pt, Bitboard table[], Magic magics[][2]) {
    int size{0};
    int total_size{0};
    for (Square s{SQ_A1}; s <= SQ_H8; ++s) {
        Bitboard edges{((Rank1BB | Rank8BB) & ~rank_bb(s)) | ((FileABB | FileHBB) & ~file_bb(s))};

        Magic& m{magics[s][pt - BISHOP]};
        m.mask = Bitboards::sliding_attack(pt, s, 0) & ~edges;

        m.attacks = s == SQ_A1 ? table : magics[s - 1][pt - BISHOP].attacks + size;
        size = 0;

        Bitboard b{0};
        do {
            m.attacks[_pext_u64(b, m.mask)] = Bitboards::sliding_attack(pt, s, b);

            ++size;
            b = (b - m.mask) & m.mask;
        } while (b);
        total_size += size;
    }
    std::cout << "Magic table size: " << 8 * total_size << " bytes\n";
}
}  // namespace Bitboards
