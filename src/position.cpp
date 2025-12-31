#include <cassert>
#include <cctype>
#include <ios>
#include <iostream>
#include <sstream>
#include "bitboard.h"
#include "position.h"
#include "pretty.h"
#include "types.h"

using namespace Bitboards;

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

    update_slider_blockers(sideToMove);
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

inline Bitboard Position::pieces() const {
    return byTypeBB[ALL_PIECES];
}

inline Bitboard Position::pieces(Color c) const {
    return byColorBB[c];
}

template <typename... PieceType>
inline Bitboard Position::pieces(PieceType... pts) const {
    return (byTypeBB[pts] | ...);
}

template <typename... PieceType>
inline Bitboard Position::pieces(Color c, PieceType... pts) const {
    return pieces(c) & pieces(pts...);
}

Piece Position::piece_on(Square s) const {
    assert(is_ok(s));
    return board[s];
}

template <PieceType Pt>
Square Position::square(Color c) const {
    return lsb(pieces(Pt, c));
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

bool Position::castling_impeded(CastlingRights cr) const {
    assert(cr == WHITE_OO || cr == WHITE_OOO || cr == BLACK_OO || cr == BLACK_OOO);
    return CastlingPaths[cr] & pieces();
}

Square Position::castling_rook_square(CastlingRights cr) const {
    assert(cr == WHITE_OO || cr == WHITE_OOO || cr == BLACK_OO || cr == BLACK_OOO);
    return CastlingRookSquares[cr];
}

bool Position::empty(Square s) const {
    return piece_on(s) == NO_PIECE;
}

// Computes a bitboard of all pieces which attack a given square.
// Slider attacks use the occupied bitboard to indicate occupancy.
Bitboard Position::attackers_to(Square s) const {
    return attackers_to(s, pieces());
}

template <PieceType... Pts>
Bitboard Position::attackers_to(Square s) const {
    return attackers_to<Pts...>(s, pieces());
}

Bitboard Position::attackers_to(Square s, Bitboard occupied) const {
    return attackers_to<KNIGHT, BISHOP, ROOK, QUEEN, KING>(s, occupied) |
           attacks_bb<PAWN>(s, WHITE) | attacks_bb<PAWN>(s, BLACK);
}

template <PieceType... Pts>
Bitboard Position::attackers_to(Square s, Bitboard occupied) const {
    return ((attacks_bb(Pts, s, occupied) & pieces(Pts)) | ...);
}

bool Position::attackers_to_exist(Square s, Bitboard occupied, Color c) const {
    return attackers_to(s, occupied) & pieces(c);
}

template <PieceType... Pts>
bool Position::attackers_to_exist(Square s, Bitboard occupied, Color c) const {
    return attackers_to<Pts...>(s, occupied) & pieces(c);
}

bool Position::legal(Move m) const {
    assert(m.is_ok());

    Square to = m.to_sq();
    Square from = m.from_sq();
    Color us = sideToMove;

    assert(color_of(moved_piece(m)) == us);

    if (m.type_of() == EN_PASSANT) {
        Square ksq = square<KING>(us);
        Square capsq = to - pawn_push(us);
        Bitboard occupied = pieces() ^ from ^ capsq | to;

        assert(to == ep_square());
        assert(moved_piece(m) == make_piece(us, PAWN));
        assert(piece_on(capsq) == make_piece(~us, PAWN));
        assert(piece_on(to) == NO_PIECE);

        return attackers_to_exist<BISHOP, ROOK, QUEEN>(ksq, occupied, ~us);
    }

    if (m.type_of() == CASTLING) {
        Direction step = from > to ? WEST : EAST;

        to = relative_square(us, from > to ? SQ_C1 : SQ_G1);

        for (Square s = to; s != from; s += step) {
            if (attackers_to_exist(s, pieces(), ~us)) {
                return false;
            }
        }
        return true;
    }

    if (type_of(moved_piece(m)) == KING && attackers_to_exist(to, pieces() ^ from, ~us)) {
        return false;
    }

    return !(blockers_for_king(us) & from || line_bb(from, to) & square<KING>(us));
}

bool Position::pseudo_legal(Move m) const {}

void Position::update_slider_blockers(Color c) {
    st.blockersForKing[c] = 0;
    st.pinners[~c] = 0;

    Square ksq = square<KING>(c);

    Bitboard snipers = attackers_to<BISHOP, ROOK, QUEEN>(ksq) & pieces(~c);
    Bitboard occupancy = pieces() ^ snipers;

    while (snipers) {
        Square sniperSq = pop_lsb(snipers);

        Bitboard b = between_bb(ksq, sniperSq) & occupancy;

        if (b && !more_than_one(b)) {
            st.blockersForKing[c] |= b;
            if (b & pieces(c)) {
                st.pinners[~c] |= sniperSq;
            }
        }
    }
}

void Position::make_move(Move move) {}

void Position::unmake_move(Move move) {}
