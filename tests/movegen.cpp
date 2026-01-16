#include <gtest/gtest.h>
#include "../src/pretty.h"
#include "positions.h"

TEST(TestMoveGeneration, NumNodesAreCorrect) {
    for (const auto& test : testPositions) {
        if (test.depth > 4)
            continue;

        Position pos{test.fen};
        int result = generate_nodes(pos, test.depth);

        ASSERT_EQ(result, test.nodes) << "Test instance:\n"
                                      << test << "\n------------------\n"
                                      << "POSITION ROOT NODE\n"
                                      << "Moves:\n"
                                      << MoveList<LEGAL>(pos) << "Board:\n"
                                      << pos;
    }
}
