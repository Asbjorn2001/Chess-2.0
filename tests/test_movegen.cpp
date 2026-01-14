#include <gtest/gtest.h>
#include "../src/pretty.h"
#include "positions.h"

int generate_nodes(Position& pos, int depth) {
    if (depth == 0) {
        return 1;
    }

    int num_positions{0};
    for (const auto& m : MoveList<LEGAL>(pos)) {
        pos.make_move(m);
        num_positions += generate_nodes(pos, depth - 1);
        pos.unmake_move(m);
    }

    return num_positions;
}

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
