#include <cassert>
#include <cctype>
#include <ios>
#include <iostream>
#include <sstream>
#include "bitboard.h"
#include "macros.h"
#include "position.h"
#include "pretty.h"
#include "types.h"

using namespace Bitboards;

Position::Position(const std::string fenStr) {
    std::istringstream ss{fenStr};
    Square sq = SQ_A8;
    st = new StateInfo{};

    char token, col, row;

    ss >> std::noskipws;

    while ((ss >> token) && !isspace(token)) {
        // Note that fen string advcances from the top left A8 to H1,
        // while the board is ordered from A1 to H8.
        if (isdigit(token)) {
            sq += (token - '0') * EAST;
        } else if (token == '/') {
            sq += 2 * SOUTH;
        } else if (Piece pc = pc_from_char(token); pc != NO_PIECE) {
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
    st->castlingRights = castling;

    if (((ss >> col) && col >= 'a' && col <= 'h') &&
        ((ss >> row) && row == (sideToMove == WHITE ? '6' : '3'))) {
        st->epSquare = make_square(File(col - 'a'), Rank(row - '1'));
    } else {
        st->epSquare = SQ_NONE;
    }

    ss >> std::skipws >> st->rule50 >> gamePly;

    gamePly = std::max(2 * (gamePly - 1), 0) + (sideToMove == BLACK);
    st->checkersBB = attackers_to(square<KING>(sideToMove)) & pieces(~sideToMove);

    update_slider_blockers(sideToMove);
}

std::string Position::as_fen() const {
    int emptyCnt{};
    std::ostringstream ss{};

    for (Rank r = RANK_8; r >= RANK_1; --r) {
        for (File f = FILE_A; f <= FILE_H; ++f) {
            for (emptyCnt = 0; f <= FILE_H && is_empty(make_square(f, r)); ++f) {
                ++emptyCnt;
            }

            if (emptyCnt) {
                ss << emptyCnt;
            }

            if (f <= FILE_H && !is_empty(make_square(f, r))) {
                ss << pc_as_char(piece_on(make_square(f, r)));
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

    ss << (st->epSquare == SQ_NONE ? " - " : " " + pretty(st->epSquare) + " ") << st->rule50 << " "
       << 1 + (gamePly - (sideToMove == BLACK)) / 2;

    return ss.str();
}

Piece Position::piece_on(Square s) const {
    assert(is_ok(s));
    return board_[s];
}

void Position::put_piece(Piece p, Square s) {
    MY_ASSERT(is_empty(s), "pc: " << p << " s: " << s << " pos:\n" << *this);
    board_[s] = p;
    byTypeBB[ALL_PIECES] |= byTypeBB[type_of(p)] |= s;
    byColorBB[color_of(p)] |= s;
}

void Position::remove_piece(Square s) {
    assert(!is_empty(s));
    Piece p = board_[s];

    byTypeBB[ALL_PIECES] ^= s;
    byTypeBB[type_of(p)] ^= s;
    byColorBB[color_of(p)] ^= s;
    board_[s] = NO_PIECE;
}

// Moves a piece from the from square, to the to square. Assumes the `to` square is empty.
void Position::move_piece(Square from, Square to) {
    MY_ASSERT(is_empty(to), "pc: " << piece_on(to));

    Piece p = board_[from];
    Bitboard fromTo = from | to;

    byTypeBB[ALL_PIECES] ^= fromTo;
    byTypeBB[type_of(p)] ^= fromTo;
    byColorBB[color_of(p)] ^= fromTo;
    board_[from] = NO_PIECE;
    board_[to] = p;
}

bool Position::can_castle(CastlingRights cr) const {
    return st->castlingRights & cr;
}

bool Position::castling_impeded(CastlingRights cr) const {
    assert(cr == WHITE_OO || cr == WHITE_OOO || cr == BLACK_OO || cr == BLACK_OOO);
    return CastlingPaths[cr] & pieces();
}

Square Position::castling_rook_square(CastlingRights cr) const {
    assert(cr == WHITE_OO || cr == WHITE_OOO || cr == BLACK_OO || cr == BLACK_OOO);
    return CastlingRookSquares[cr];
}

bool Position::is_empty(Square s) const {
    return piece_on(s) == NO_PIECE;
}

bool Position::legal(Move m) const {
    assert(m.is_ok());

    Square to = m.to_sq();
    Square from = m.from_sq();
    Color us = sideToMove;
    Color them = ~us;

    MY_ASSERT(moved_piece(m) != NO_PIECE, "pos:\n" << *this << "m: " << m);
    MY_ASSERT(color_of(moved_piece(m)) == us,
              "m: " << m << " us: " << us << " pc: " << moved_piece(m));

    if (m.type_of() == EN_PASSANT) {
        Square ksq = square<KING>(us);
        Square capsq = to - pawn_push(us);
        Bitboard occupied = (pieces() ^ from ^ capsq) | to;

        assert(to == ep_square());
        assert(moved_piece(m) == make_piece(us, PAWN));
        assert(piece_on(capsq) == make_piece(them, PAWN));
        assert(piece_on(to) == NO_PIECE);

        return !attackers_to_exist<BISHOP, ROOK, QUEEN>(ksq, occupied, them);
    }

    if (m.type_of() == CASTLING) {
        Direction step = from > to ? WEST : EAST;

        to = relative_square(us, from > to ? SQ_C1 : SQ_G1);

        for (Square s = to; s != from; s -= step) {
            if (attackers_to_exist(s, pieces(), them)) {
                return false;
            }
        }
        return true;
    }

    if (type_of(moved_piece(m)) == KING && attackers_to_exist(to, pieces() ^ from, them)) {
        return false;
    }

    return !(blockers_for_king(us) & from) || line_bb(from, to) & square<KING>(us);
}

bool Position::pseudo_legal(Move m) const {}

void Position::update_slider_blockers(Color c) {
    st->blockersForKing[c] = 0;
    st->pinners[~c] = 0;

    Square ksq = square<KING>(c);

    Bitboard snipers = attackers_to<BISHOP, ROOK, QUEEN>(ksq, 0) & pieces(~c);
    Bitboard occupancy = pieces() ^ snipers;

    while (snipers) {
        Square sniperSq = pop_lsb(snipers);

        Bitboard b = between_bb(ksq, sniperSq) & occupancy;

        if (b && !more_than_one(b)) {
            st->blockersForKing[c] |= b;
            if (b & pieces(c)) {
                st->pinners[~c] |= sniperSq;
            }
        }
    }
}

void Position::make_move(Move m) {
    assert(legal(m));

    Square from = m.from_sq();
    Square to = m.to_sq();
    Color us = sideToMove;
    Color them = ~us;

    // Init and set a new state
    StateInfo* newState = new StateInfo{*st};
    newState->rule50 = st->rule50 + 1;
    newState->previous = st;
    st = newState;
    st->capturedPiece = NO_PIECE;
    st->epSquare = SQ_NONE;

    if (!is_empty(to) && m.type_of() != EN_PASSANT && m.type_of() != CASTLING) {
        st->capturedPiece = piece_on(to);
        remove_piece(to);
    }

    // Check and handle double pawn pushes
    if (type_of(moved_piece(m)) == PAWN) {
        if ((rank_of(from) == relative_rank(us, RANK_2)) &&
            (rank_of(to) == relative_rank(us, RANK_4))) {
            Square epTarget = from + pawn_push(us);
            st->epSquare = (attackers_to(epTarget) & pieces<PAWN>(them)) ? epTarget : SQ_NONE;
        }
    }

    if (m.type_of() == EN_PASSANT) {
        assert(moved_piece(m) == make_piece(us, PAWN));
        assert(piece_on(to) == NO_PIECE);
        assert(rank_of(to) == relative_rank(us, RANK_6));

        Square capsq = to - pawn_push(us);
        st->capturedPiece = piece_on(capsq);

        remove_piece(capsq);
        move_piece(from, to);
    } else if (m.type_of() == CASTLING) {
        assert(moved_piece(m) == make_piece(us, KING));
        assert(piece_on(to) == make_piece(us, ROOK));

        Direction step = from > to ? WEST : EAST;

        // King moves 2 steps towards the rook
        Square ksq = from + 2 * step;
        move_piece(from, ksq);

        // Rook is placed on the opposite side of the king
        Square rsq = ksq - step;
        move_piece(to, rsq);
    } else if (m.type_of() == PROMOTION) {
        assert(moved_piece(m) == make_piece(us, PAWN));
        assert(rank_of(to) == relative_rank(us, RANK_8));
        assert(m.promotion_type() != NO_PIECE_TYPE);

        Piece p = make_piece(us, m.promotion_type());

        // Manually move and replace the pawn, to ensure the correct type is placed.
        remove_piece(from);
        put_piece(p, to);
    } else {
        move_piece(from, to);
    }

    // Remove castling rights if any key squares are affected
    if (CastlingSquares & (from | to)) {
        remove_castling_rights(cr_from_sq(from));
        remove_castling_rights(cr_from_sq(to));
    }

    // Update state
    update_slider_blockers(WHITE);
    update_slider_blockers(BLACK);
    st->checkersBB = attackers_to(square<KING>(them)) & pieces(us);

    ++gamePly;
    sideToMove = them;
}

void Position::unmake_move(Move m) {
    assert(m.is_ok());

    Square from = m.from_sq();
    Square to = m.to_sq();
    Color us = ~sideToMove;

    MY_ASSERT(m.type_of() == CASTLING || piece_on(to) != NO_PIECE, "pos:\n" << *this << "m: " << m);

    if (m.type_of() == EN_PASSANT) {
        move_piece(to, from);
        put_piece(st->capturedPiece, to - pawn_push(us));
    } else if (m.type_of() == CASTLING) {
        Direction step = from > to ? WEST : EAST;
        Square ksq = from + 2 * step;
        move_piece(ksq, from);

        Square rsq = ksq - step;
        move_piece(rsq, to);
    } else if (m.type_of() == PROMOTION) {
        remove_piece(to);
        put_piece(make_piece(us, PAWN), from);
    } else {
        move_piece(to, from);
    }

    if (st->capturedPiece != NO_PIECE && m.type_of() != EN_PASSANT) {
        put_piece(st->capturedPiece, to);
    }

    StateInfo* si = st;
    st = st->previous;
    delete si;

    --gamePly;
    sideToMove = us;
}
