#include <gtest/gtest.h>
#include "../src/bitboard.h"

int main(int argc, char** argv) {
    Bitboards::init();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
