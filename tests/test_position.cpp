#include <gtest/gtest.h>
#include "../src/pretty.h"

class TestPosition : public testing::Test {
   protected:
    TestPosition()
        : position1("r6r/1b2k1bq/8/8/7B/6pP/8/R3K2R b KQ - 3 2"),
          position2("8/8/8/2k5/2pP4/8/B7/4K3 b - d3 0 3"),
          position3("rnb2k1r/pp1Pbppp/2p5/q7/2B5/8/PPPQNnPP/RNB1K2R w KQ - 3 9") {}

    Position position1;
    Position position2;
    Position position3;
};

void testPieces(Bitboard result, Bitboard expected) {
    ASSERT_EQ(result, expected) << "Expected:\n"
                                << pretty(expected) << "Result:\n"
                                << pretty(result);
}

TEST_F(TestPosition, PiecesWorks) {
    testPieces(position1.pieces<PAWN>(WHITE), square_bb(SQ_H3));
    testPieces(position1.pieces<KING, QUEEN>(BLACK), SQ_H7 | SQ_E7);
    testPieces(position1.pieces<ROOK>(), SQ_H8 | SQ_H1 | SQ_A8 | SQ_A1);
    testPieces(position1.pieces(WHITE), SQ_H4 | SQ_H3 | SQ_H1 | SQ_E1 | SQ_A1);
}

TEST_F(TestPosition, LegalMoveWorks) {
    ASSERT_TRUE(position1.legal(Move(SQ_G7, SQ_F6)));
    ASSERT_TRUE(position1.legal(Move(SQ_H7, SQ_H4)));
    ASSERT_FALSE(position1.legal(Move(SQ_E7, SQ_D8)));

    ASSERT_TRUE(position2.legal(Move::make<EN_PASSANT>(SQ_C4, SQ_D3)));

    ASSERT_TRUE(position3.legal(Move::make<CASTLING>(SQ_E1, SQ_H1)));
}
