#include <cstdlib>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include "src/MoveGenerator.h"

void cli_game_loop(ChessBoard& board);

int main(int argc, char *argv[]) {
    try {
        auto chessboard = ChessBoard();

        cli_game_loop(chessboard);

    } catch(std::logic_error e) {
        std::cout << e.what() << "\n";
    }

    return 0;
}

void cli_game_loop(ChessBoard& board) {
    auto move_generator = MoveGenerator();

    while (true) {
        std::cout << board << "\n\n";
        std::cout << board.as_fen_string() << "\n";
        auto moves = move_generator.generate_legal_moves(board);

        auto count = 1;
        for (const auto& move : moves) {
            std::cout << count++ << ") " << move << "\n";
        }

        std::cout << "Choose a move: \n";
        size_t move_num{0};
        std::cin >> move_num;
        std::cout << "your choose: " << move_num << "\n";

        board.move(moves[move_num - 1]);
    }
}
