#include <cstdlib>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include "src/MoveGenerator.h"

int main(int argc, char *argv[]) {
    try {
        auto chessboard = ChessBoard();
        std::cout << chessboard << "\n\n";

        auto fen_string = chessboard.as_fen_string();
        std::cout << fen_string;

        auto move_generator = MoveGenerator();
        auto moves = move_generator.generate_legal_moves(chessboard);

        for (const auto& move : moves) {
            std::cout << move << "\n";
        }
    } catch(std::logic_error e) {
        std::cout << e.what() << "\n";
    }

    return 0;
}
