#include <gtest/gtest.h>
#include <cerrno>
#include "../src/pretty.h"

constexpr auto testPosition = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";

void test_pseudo_attacks_helper(PieceType pt,
                                Square s,
                                const Position& p,
                                Bitboard result,
                                Bitboard expected) {
    EXPECT_EQ(result, expected) << "Pseudo attacks test for " << pt << " on " << s
                                << " using position: \n"
                                << p << "Expected: \n"
                                << pretty(expected) << "Actual: \n"
                                << pretty(result);
}

TEST(bitboard_test, pseudo_attacks) {
    Bitboards::init();

    Position p{testPosition};
    Bitboard occupied = p.pieces();

    test_pseudo_attacks_helper(ROOK, SQ_H1, p, attacks_bb(ROOK, SQ_H1, occupied),
                               SQ_E1 | SQ_F1 | SQ_G1 | SQ_H2);

    test_pseudo_attacks_helper(QUEEN, SQ_D1, p, attacks_bb(QUEEN, SQ_D1, occupied),
                               (FileDBB ^ (SQ_D8 | SQ_D1)) | SQ_C1 | SQ_E1 | SQ_C2 | SQ_E2);
}
