#include <iostream>
#include "bitboard.h"
#include "gtest/gtest.h"
#include "movegen.h"
#include "position.h"
#include "pretty.h"
#include "types.h"

Move* splat_moves(Move* moveList, Square from, Bitboard to_bb) {
    while (to_bb) {
        *moveList++ = Move(from, pop_lsb(to_bb));
    }
    return moveList;
}

template <Direction offset>
inline Move* splat_pawn_moves(Move* moveList, Bitboard to_bb) {
    while (to_bb) {
        Square to = pop_lsb(to_bb);
        *moveList++ = Move(to - offset, to);
    }
    return moveList;
}

template <GenType Type, Direction D, bool Enemy>
Move* make_promotions(Move* moveList, [[maybe_unused]] Square to) {
    constexpr bool all = Type == EVASIONS || Type == NON_EVASIONS;

    if constexpr (Type == TACTICALS || all) {
        *moveList++ = Move::make<PROMOTION>(to - D, to, QUEEN);
    }

    if constexpr ((Type == TACTICALS && Enemy) || (Type == QUIETS && !Enemy) || all) {
        *moveList++ = Move::make<PROMOTION>(to - D, to, ROOK);
        *moveList++ = Move::make<PROMOTION>(to - D, to, BISHOP);
        *moveList++ = Move::make<PROMOTION>(to - D, to, KNIGHT);
    }

    return moveList;
}

template <GenType Type, Color Us>
Move* generate_pawn_moves(const Position& pos, Move* moveList, Bitboard target) {
    constexpr Color Them = ~Us;
    constexpr Bitboard RelRank7BB = Us == WHITE ? Rank7BB : Rank2BB;
    constexpr Bitboard RelRank3BB = Us == WHITE ? Rank3BB : Rank6BB;
    constexpr Direction Up = pawn_push(Us);
    constexpr Direction UpRight = Us == WHITE ? NORTH_EAST : SOUTH_WEST;
    constexpr Direction UpLeft = Us == WHITE ? NORTH_WEST : SOUTH_EAST;

    Bitboard enemies = Type == EVASIONS ? pos.checkers() : pos.pieces(Them);
    Bitboard emptySquares = ~pos.pieces();

    Bitboard pawnsOn7 = pos.pieces(Us, PAWN) & RelRank7BB;
    Bitboard pawnsNotOn7 = pos.pieces(Us, PAWN) & ~RelRank7BB;

    if constexpr (Type != TACTICALS) {
        Bitboard b1 = shift<Up>(pawnsNotOn7) & emptySquares;
        Bitboard b2 = shift<Up>(b1 & RelRank3BB) & emptySquares;

        if constexpr (Type == EVASIONS) {
            b1 &= target;
            b2 &= target;
        }

        moveList = splat_pawn_moves<Up>(moveList, b1);
        moveList = splat_pawn_moves<Up + Up>(moveList, b2);
    }

    // Promotions and underpromotions
    if (pawnsOn7) {
        Bitboard b1 = shift<UpRight>(pawnsOn7) & enemies;
        Bitboard b2 = shift<UpLeft>(pawnsOn7) & enemies;
        Bitboard b3 = shift<Up>(pawnsOn7) & emptySquares;

        if constexpr (Type == EVASIONS) {
            b3 &= target;
        }

        while (b1) {
            moveList = make_promotions<Type, UpRight, true>(moveList, pop_lsb(b1));
        }

        while (b2) {
            moveList = make_promotions<Type, UpLeft, true>(moveList, pop_lsb(b2));
        }

        while (b3) {
            moveList = make_promotions<Type, Up, false>(moveList, pop_lsb(b3));
        }
    }

    // Standard and en passant captures
    if constexpr (Type == TACTICALS || Type == EVASIONS || Type == NON_EVASIONS) {
        Bitboard b1 = shift<UpRight>(pawnsNotOn7) & enemies;
        Bitboard b2 = shift<UpLeft>(pawnsNotOn7) & enemies;

        moveList = splat_pawn_moves<UpRight>(moveList, b1);
        moveList = splat_pawn_moves<UpLeft>(moveList, b2);

        if (pos.ep_square() != SQ_NONE) {
            assert(rank_of(pos.ep_square()) == relative_rank(Us, RANK_6));

            // An en passant capture cannot resolve a discovered check
            if (Type == EVASIONS && (target & (pos.ep_square() + Up))) {
                return moveList;
            }

            b1 = pawnsNotOn7 & attacks_bb<PAWN>(pos.ep_square(), Them);

            assert(b1);

            while (b1) {
                *moveList++ = Move::make<EN_PASSANT>(pop_lsb(b1), pos.ep_square());
            }
        }
    }

    return moveList;
}

template <PieceType Pt, Color Us>
Move* generate_moves(const Position& pos, Move* moveList, Bitboard target) {
    static_assert(Pt != PAWN && Pt != KING, "Unsupported type in generate_moves()");

    Bitboard pieces = pos.pieces(Us, Pt);
    Bitboard occupied = pos.pieces();

    while (pieces) {
        Square from = pop_lsb(pieces);

        Bitboard to_bb = attacks_bb(Pt, from, occupied) & target;

        moveList = splat_moves(moveList, from, to_bb);
    }

    return moveList;
}

template <GenType Type, Color Us>
Move* generate_all(const Position& pos, Move* moveList) {
    static_assert(Type != LEGAL, "Unsupported type in generate_all()");

    const Square ksq = pos.square<KING>(Us);
    Bitboard target{};

    if (Type != EVASIONS || !more_than_one(pos.checkers())) {
        target = Type == EVASIONS       ? between_bb(ksq, lsb(pos.checkers()))
                 : Type == NON_EVASIONS ? ~pos.pieces(Us)
                 : Type == TACTICALS    ? pos.pieces(~Us)
                                        : ~pos.pieces();  // Quiets

        moveList = generate_pawn_moves<Type, Us>(pos, moveList, target);
        moveList = generate_moves<KNIGHT, Us>(pos, moveList, target);
        moveList = generate_moves<BISHOP, Us>(pos, moveList, target);
        moveList = generate_moves<ROOK, Us>(pos, moveList, target);
        moveList = generate_moves<QUEEN, Us>(pos, moveList, target);
    }

    Bitboard b = attacks_bb<KING>(ksq) & (Type == EVASIONS ? ~pos.pieces(Us) : target);

    moveList = splat_moves(moveList, ksq, b);

    if ((Type == QUIETS || Type == NON_EVASIONS) && pos.can_castle(Us & ANY_CASTLING))
        for (CastlingRights cr : {Us & KING_SIDE, Us & QUEEN_SIDE})
            if (!pos.castling_impeded(cr) && pos.can_castle(cr))
                *moveList++ = Move::make<CASTLING>(ksq, pos.castling_rook_square(cr));

    return moveList;
}
// <TACTICALS>    Generates all pseudo-legal captures plus queen promotions
// <QUIETS>       Generates all pseudo-legal non-captures and underpromotions
// <EVASIONS>     Generates all pseudo-legal check evasions
// <NON_EVASIONS> Generates all pseudo-legal captures and non-captures
//
// Returns a pointer to the end of the move list.
template <GenType Type>
Move* generate(const Position& pos, Move* moveList) {
    static_assert(Type != LEGAL, "Unsupported type in generate()");
    assert((Type == EVASIONS) == bool(pos.checkers()));

    Color us = pos.side_to_move();

    return us == WHITE ? generate_all<Type, WHITE>(pos, moveList)
                       : generate_all<Type, BLACK>(pos, moveList);
}

// Explicit template instantiations
template Move* generate<TACTICALS>(const Position&, Move*);
template Move* generate<QUIETS>(const Position&, Move*);
template Move* generate<EVASIONS>(const Position&, Move*);
template Move* generate<NON_EVASIONS>(const Position&, Move*);

// generate<LEGAL> generates all the legal moves in the given position

template <>
Move* generate<LEGAL>(const Position& pos, Move* moveList) {
    Color us = pos.side_to_move();
    Bitboard pinned = pos.blockers_for_king(us) & pos.pieces(us);
    Square ksq = pos.square<KING>(us);
    Move* cur = moveList;

    moveList =
        pos.checkers() ? generate<EVASIONS>(pos, moveList) : generate<NON_EVASIONS>(pos, moveList);
    while (cur != moveList) {
        if (((pinned & cur->from_sq()) || cur->from_sq() == ksq || cur->type_of() == EN_PASSANT) &&
            !pos.legal(*cur)) {
            *cur = *(--moveList);
        } else {
            ++cur;
        }
    }

    return moveList;
}
