#include <cassert>
#include <cctype>
#include <ios>
#include <iostream>
#include <sstream>
#include "pretty.h"

namespace Position {

Position::Position(const std::string fenStr) {
    std::istringstream ss{fenStr};
    Square sq = SQ_A8;

    char token, col, row;

    ss >> std::noskipws;

    while ((ss >> token) && !isspace(token)) {
        // Note that fen string advcances from the top left A8 to H1,
        // while the board is ordered from A1 to H8.
        if (isdigit(token)) {
            sq += (token - '0') * EAST;
        } else if (token == '/') {
            sq += 2 * SOUTH;
        } else if (Piece pc = from_char(token); pc != NO_PIECE) {
            put_piece(pc, sq);
            ++sq;
        }
    }

    ss >> token;
    sideToMove = token == 'w' ? WHITE : BLACK;
    ss >> token;

    CastlingRights castling = NO_CASTLING;
    while ((ss >> token) && !isspace(token)) {
        Color c = isupper(token) ? WHITE : BLACK;

        token = static_cast<char>(toupper(token));
        if (token == 'K') {
            castling |= (c == WHITE ? WHITE_OO : BLACK_OO);
        } else if (token == 'Q') {
            castling |= (c == WHITE ? WHITE_OOO : BLACK_OOO);
        }
    }
    st.castlingRights = castling;

    if (((ss >> col) && col >= 'a' && col <= 'h') &&
        ((ss >> row) && row == (sideToMove == WHITE ? '6' : '3'))) {
        st.epSquare = make_square(File(col - 'a'), Rank(row - '1'));
    } else {
        st.epSquare = SQ_NONE;
    }

    ss >> std::skipws >> st.rule50 >> gamePly;

    gamePly = std::max(2 * (gamePly - 1), 0) + (sideToMove == BLACK);
}

std::string Position::as_fen() const {
    int emptyCnt{};
    std::ostringstream ss{};

    for (Rank r = RANK_8; r >= RANK_1; --r) {
        for (File f = FILE_A; f <= FILE_H; ++f) {
            for (emptyCnt = 0; f <= FILE_H && empty(make_square(f, r)); ++f) {
                ++emptyCnt;
            }

            if (emptyCnt) {
                ss << emptyCnt;
            }

            if (f <= FILE_H && !empty(make_square(f, r))) {
                ss << as_char(piece_on(make_square(f, r)));
            }
        }

        if (r > RANK_1)
            ss << '/';
    }

    ss << (sideToMove == WHITE ? " w " : " b ");

    if (can_castle(WHITE_OO))
        ss << 'K';

    if (can_castle(WHITE_OOO))
        ss << 'Q';

    if (can_castle(BLACK_OO))
        ss << 'k';

    if (can_castle(BLACK_OOO))
        ss << 'q';

    if (!can_castle(ANY_CASTLING))
        ss << '-';

    ss << (st.epSquare == SQ_NONE ? " - " : " " + pretty(st.epSquare) + " ") << st.rule50 << " "
       << 1 + (gamePly - (sideToMove == BLACK)) / 2;

    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const Position& p) {
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

    return os << s;
}

Bitboard Position::pieces() const {
    return byTypeBB[ALL_PIECES];
}

Bitboard Position::pieces(Color c) const {
    return byColorBB[c];
}

template <typename... PieceType>
Bitboard Position::pieces(PieceType... pts) const {
    return (byTypeBB[pts] | ...);
}

template <typename... PieceType>
Bitboard Position::pieces(Color c, PieceType... pts) const {
    return pieces(c) & pieces(pts...);
}

Piece Position::piece_on(Square s) const {
    assert(is_ok(s));
    return board[s];
}

void Position::put_piece(Piece p, Square s) {
    board[s] = p;
    byTypeBB[ALL_PIECES] |= byTypeBB[type_of(p)] |= s;
    byColorBB[color_of(p)] |= s;
}

void Position::remove_piece(Square s) {
    Piece p = board[s];

    byTypeBB[ALL_PIECES] ^= s;
    byTypeBB[type_of(p)] ^= s;
    byColorBB[color_of(p)] ^= s;
    board[s] = NO_PIECE;
}

void Position::move_piece(Square from, Square to) {
    Piece p = board[from];
    Bitboard fromTo = from | to;

    byTypeBB[ALL_PIECES] ^= fromTo;
    byTypeBB[type_of(p)] ^= fromTo;
    byColorBB[color_of(p)] ^= fromTo;
    board[from] = NO_PIECE;
    board[to] = p;
}

bool Position::can_castle(CastlingRights cr) const {
    return st.castlingRights & cr;
}

bool Position::empty(Square s) const {
    return piece_on(s) == NO_PIECE;
}

}  // namespace Position
