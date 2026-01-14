#include <gtest/gtest.h>
#include <cerrno>
#include "../src/pretty.h"

class TestBitboard : public testing::Test {
   protected:
    TestBitboard() : testPosition("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8") {}

    Position testPosition;
};

void test_attacks(Piece pc, Square s, Bitboard occupied, Bitboard expected) {
    Bitboard result = attacks_bb(pc, s, occupied);
    EXPECT_EQ(result, expected) << "Pseudo attacks test for " << pc << " on " << s
                                << " with occupation:\n"
                                << pretty(occupied) << "Expected:\n"
                                << pretty(expected) << "Actual:\n"
                                << pretty(result);
}

TEST_F(TestBitboard, AttacksBBWorks) {
    Bitboard occupied = testPosition.pieces();
    test_attacks(W_ROOK, SQ_H1, occupied, SQ_E1 | SQ_F1 | SQ_G1 | SQ_H2);

    test_attacks(W_QUEEN, SQ_D1, occupied,
                 (FileDBB ^ (SQ_D8 | SQ_D1)) | SQ_C1 | SQ_E1 | SQ_C2 | SQ_E2);
    test_attacks(B_BISHOP, SQ_C4, occupied,
                 SQ_D3 | SQ_E2 | SQ_B3 | SQ_A2 | SQ_D5 | SQ_E6 | SQ_F7 | SQ_B5 | SQ_A6);
    test_attacks(W_BISHOP, SQ_C1, occupied, SQ_D2 | SQ_E3 | SQ_F4 | SQ_G5 | SQ_H6 | SQ_B2);
    test_attacks(W_PAWN, SQ_A2, occupied, square_bb(SQ_B3));
    test_attacks(W_PAWN, SQ_E8, occupied, 0);
    test_attacks(W_PAWN, SQ_D4, occupied, SQ_C5 | SQ_E5);
    test_attacks(B_PAWN, SQ_C5, occupied, SQ_B4 | SQ_D4);

    test_attacks(W_KING, SQ_E1, occupied, SQ_D1 | SQ_F1 | SQ_D2 | SQ_E2 | SQ_F2);
    test_attacks(W_KNIGHT, SQ_E2, occupied, SQ_C1 | SQ_G1 | SQ_G3 | SQ_F4 | SQ_D4 | SQ_C3);
    test_attacks(B_KNIGHT, SQ_D4, occupied,
                 SQ_E2 | SQ_F3 | SQ_F5 | SQ_E6 | SQ_C6 | SQ_B5 | SQ_B3 | SQ_C2);
    test_attacks(B_KNIGHT, SQ_F2, occupied, SQ_D1 | SQ_H1 | SQ_H3 | SQ_G4 | SQ_E4 | SQ_D3);
    test_attacks(B_KNIGHT, SQ_E1, occupied, SQ_C2 | SQ_G2 | SQ_D3 | SQ_F3);
}

void test_between(Square s1, Square s2, Bitboard expected) {
    Bitboard result = between_bb(s1, s2);
    EXPECT_EQ(result, expected) << "Between test from " << s1 << " to " << s2 << "\nExpected:\n"
                                << pretty(expected) << "Actual:\n"
                                << pretty(result);
}

TEST_F(TestBitboard, BetweenBBWorks) {
    test_between(SQ_C2, SQ_C4, SQ_C3 | SQ_C4);
    test_between(SQ_C6, SQ_B2, square_bb(SQ_B2));
    test_between(SQ_A1, SQ_H8, SQ_B2 | SQ_C3 | SQ_D4 | SQ_E5 | SQ_F6 | SQ_G7 | SQ_H8);
}

void test_line(Square s1, Square s2, Bitboard expected) {
    Bitboard result = line_bb(s1, s2);
    EXPECT_EQ(result, expected) << "Between test from " << s1 << " to " << s2 << "\nExpected:\n"
                                << pretty(expected) << "Actual:\n"
                                << pretty(result);
}

TEST_F(TestBitboard, LinesBBWorks) {
    test_line(SQ_C4, SQ_F7, SQ_A2 | SQ_B3 | SQ_C4 | SQ_D5 | SQ_E6 | SQ_F7 | SQ_G8);
    test_line(SQ_A1, SQ_A3, FileABB);
    test_line(SQ_B6, SQ_E6, Rank6BB);
    test_line(SQ_E4, SQ_A2, 0);
}
