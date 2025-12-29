#include <gtest/gtest.h>
#include "../src/types.h"

TEST(move_type_test, promotion_move) {
    const auto m{Move::make<PROMOTION>(SQ_A2, SQ_A1, ROOK)};

    EXPECT_EQ(m.promotion_type(), ROOK);
    EXPECT_EQ(m.type_of(), PROMOTION);
}

TEST(move_type_test, from_to_squares) {
    const Move m{SQ_A5, SQ_H5};

    EXPECT_EQ(m.from_sq(), SQ_A5);
    EXPECT_EQ(m.to_sq(), SQ_H5);
}

TEST(square_type_test, get_rank_file) {
    const Square s{SQ_B6};

    EXPECT_EQ(rank_of(s), RANK_6);
    EXPECT_EQ(file_of(s), FILE_B);
}

TEST(square_type_test, rank_file_flip) {
    const Square s{SQ_C4};

    EXPECT_EQ(flip_file(s), SQ_F4);
    EXPECT_EQ(flip_rank(s), SQ_C5);
}

TEST(piece_type_test, get_piece_data) {
    const Piece p{W_BISHOP};

    EXPECT_EQ(type_of(p), BISHOP);
    EXPECT_EQ(color_of(p), WHITE);
}

TEST(piece_type_test, swap_color) {
    const Piece p{B_KING};

    EXPECT_EQ(color_of(~p), WHITE);
}

TEST(piece_type_test, make_piece) {
    const auto p{make_piece(WHITE, KNIGHT)};

    EXPECT_EQ(p, W_KNIGHT);
}

