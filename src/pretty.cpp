#include "pretty.h"

std::string pretty(Square s) {
    assert(is_ok(s));
    return {char('a' + file_of(s)), char('1' + rank_of(s))};
}
