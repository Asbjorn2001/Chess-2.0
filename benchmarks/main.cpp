#include <benchmark/benchmark.h>
#include "../src/bitboard.h"
#include "positions.h"

BENCHMARK_REGISTER_F(PositionFixture, MoveGeneration)->DenseRange(0, BenchmarkPositions.size() - 1);

int main(int argc, char** argv) {
    Bitboards::init();
    benchmark ::MaybeReenterWithoutASLR(argc, argv);
    char arg0_default[] = "benchmark";
    char* args_default = reinterpret_cast<char*>(arg0_default);
    if (!argv) {
        argc = 1;
        argv = &args_default;
    }
    ::benchmark ::Initialize(&argc, argv);
    if (::benchmark ::ReportUnrecognizedArguments(argc, argv))
        return 1;
    ::benchmark ::RunSpecifiedBenchmarks();
    ::benchmark ::Shutdown();
    return 0;
}

// BENCHMARK_MAIN();
