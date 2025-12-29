#include <gtest/gtest.h>
#include "../src/bitboard.h"

TEST(bitboard_test, pseudo_attacks) {
    Bitboards::init();
    Bitboard rook_attacks{Bitboards::attacks_bb(ROOK, SQ_A1, 0)};

    EXPECT_EQ(rook_attacks,
              (Bitboards::FileABB | Bitboards::Rank1BB) & ~Bitboards::square_bb(SQ_A1))
        << "rook attacks:\n"
        << Bitboards::pretty(rook_attacks);
}
