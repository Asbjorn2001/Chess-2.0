#include <sstream>
#include "pretty.h"
#include "types.h"

std::string pretty(Square s) {
    assert(is_ok(s));
    return {char('a' + file_of(s)), char('1' + rank_of(s))};
}

std::string pretty(const Position& p) {
    std::string s = "+---+---+---+---+---+---+---+---+\n";

    for (Rank r = RANK_8; r >= RANK_1; --r) {
        for (File f = FILE_A; f <= FILE_H; ++f) {
            s += "  ";
            s += as_char(p.piece_on(make_square(f, r)));
            s += " ";
        }

        s += "| " + std::to_string(1 + r) + "\n+---+---+---+---+---+---+---+---+\n";
    }
    s += "  a   b   c   d   e   f   g   h\n";

    return s;
}

// Returns an ASCII representation of a bitboard suitable
// to be printed to standard output. Useful for debugging.
std::string pretty(Bitboard b) {
    std::string s = "+---+---+---+---+---+---+---+---+\n";

    for (Rank r = RANK_8; r >= RANK_1; --r) {
        for (File f = FILE_A; f <= FILE_H; ++f)
            s += b & make_square(f, r) ? "| X " : "|   ";

        s += "| " + std::to_string(1 + r) + "\n+---+---+---+---+---+---+---+---+\n";
    }
    s += "  a   b   c   d   e   f   g   h\n";

    return s;
}

std::string pretty(const Move m) {
    return m.type_of() == CASTLING ? "O-O" : pretty(m.from_sq()) + "->" + pretty(m.to_sq());
}

std::ostream& operator<<(std::ostream& os, const Position& p) {
    return os << pretty(p);
}

std::ostream& operator<<(std::ostream& os, Square s) {
    return os << pretty(s);
}

std::ostream& operator<<(std::ostream& os, Piece p) {
    return os << as_char(p);
}

std::ostream& operator<<(std::ostream& os, PieceType pt) {
    return os << as_char(Piece(pt));
}

std::ostream& operator<<(std::ostream& os, Move m) {
    return os << pretty(m);
}
