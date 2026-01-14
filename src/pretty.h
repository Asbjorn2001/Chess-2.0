#pragma once

#include <ostream>
#include <string>
#include "movegen.h"

std::string pretty(const Square s);
std::string pretty(const Bitboard b);
std::string pretty(const Position& p);
std::string pretty(const Move m);
std::string pretty(const StateInfo& st);

template <GenType T>
std::string pretty(const MoveList<T>& moveList) {
    std::string s = "";
    int counter = 0;
    for (const auto& m : moveList) {
        s += std::to_string(++counter) + ") " + pretty(m) + "\n";
    }
    return s;
}

std::ostream& operator<<(std::ostream& os, Square s);
std::ostream& operator<<(std::ostream& os, const Position& p);
std::ostream& operator<<(std::ostream& os, Piece p);
std::ostream& operator<<(std::ostream& os, PieceType pt);
std::ostream& operator<<(std::ostream& os, Move m);
std::ostream& operator<<(std::ostream& os, Color c);
std::ostream& operator<<(std::ostream& os, const StateInfo& st);

template <GenType T>
std::ostream& operator<<(std::ostream& os, const MoveList<T>& moveList) {
    return os << pretty(moveList);
}
