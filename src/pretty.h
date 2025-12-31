#pragma once

#include <ostream>
#include <string>
#include "position.h"

std::string pretty(const Square s);
std::string pretty(const Bitboard b);
std::string pretty(const Position& p);
std::string pretty(const Move m);

std::ostream& operator<<(std::ostream& os, Square s);
std::ostream& operator<<(std::ostream& os, const Position& p);
std::ostream& operator<<(std::ostream& os, Piece p);
std::ostream& operator<<(std::ostream& os, PieceType pt);
std::ostream& operator<<(std::ostream& os, Move m);
