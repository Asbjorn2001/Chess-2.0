#include <format>
#include <iterator>
#include <sstream>
#include <string>
#include "pretty.h"
#include "types.h"

std::string pretty(Square s) {
    if (!is_ok(s))
        return "no square";
    return {char('a' + file_of(s)), char('1' + rank_of(s))};
}

std::string pretty(const Position& p) {
    std::string s = "+---+---+---+---+---+---+---+---+\n";

    for (Rank r = RANK_8; r >= RANK_1; --r) {
        for (File f = FILE_A; f <= FILE_H; ++f) {
            s += "  ";
            s += pc_as_char(p.piece_on(make_square(f, r)));
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
    return pretty(m.from_sq()) + "->" + pretty(m.to_sq()) + " " +
           (m.type_of() == CASTLING     ? "(O-O)"
            : m.type_of() == PROMOTION  ? std::format("({})", pc_as_char(Piece(m.promotion_type())))
            : m.type_of() == EN_PASSANT ? "(EP)"
                                        : "");
}

std::string pretty(const StateInfo& st) {
    std::string s = "Checkers:\n";
    s += pretty(st.checkersBB);
    s += "White pinners:\n";
    s += pretty(st.pinners[WHITE]);
    s += "Black pinners:\n";
    s += pretty(st.pinners[BLACK]);
    s += "White blockers for king:\n";
    s += pretty(st.blockersForKing[WHITE]);
    s += "Black blockers for king:\n";
    s += pretty(st.blockersForKing[BLACK]);
    s += "Ep square: " + pretty(st.epSquare) + "\n";
    s += "Captured piece: " + std::string{pc_as_char(st.capturedPiece)} + "\n";

    return s;
}

std::ostream& operator<<(std::ostream& os, const Position& p) {
    return os << pretty(p);
}

std::ostream& operator<<(std::ostream& os, Square s) {
    return os << pretty(s);
}

std::ostream& operator<<(std::ostream& os, Piece p) {
    return os << pc_as_char(p);
}

std::ostream& operator<<(std::ostream& os, PieceType pt) {
    return os << pc_as_char(Piece(pt));
}

std::ostream& operator<<(std::ostream& os, Move m) {
    return os << pretty(m);
}

std::ostream& operator<<(std::ostream& os, Color c) {
    return os << (c == WHITE ? "w" : "b");
}

std::ostream& operator<<(std::ostream& os, const StateInfo& st) {
    return os << pretty(st);
}
