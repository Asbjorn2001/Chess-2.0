#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include "position.h"
#include "types.h"

constexpr int MAX_MOVES = 256;

enum GenType {
    TACTICALS,
    QUIETS,
    EVASIONS,
    NON_EVASIONS,
    LEGAL,
};

template <GenType>
Move* generate(const Position&, Move* moveList);

template <GenType T>
struct MoveList {
    explicit MoveList(const Position& pos) : last(generate<T>(pos, moveList)) {}
    const Move* begin() const { return moveList; }
    const Move* end() const { return last; }
    Move& operator[](size_t index) {
        assert(index <= size());
        return *(moveList + index);
    }
    size_t size() const { return last - moveList; }
    bool contains(Move move) const { return std::find(begin(), end(), move) != end(); }

   private:
    Move moveList[MAX_MOVES], *last;
};
