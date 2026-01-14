#pragma once

// Source - https://stackoverflow.com/a
// Posted by Konrad Rudolph, modified by community. See post 'Timeline' for change history
// Retrieved 2026-01-11, License - CC BY-SA 3.0

#ifndef NDEBUG
#define MY_ASSERT(condition, message)                                                    \
    do {                                                                                 \
        if (!(condition)) {                                                              \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ << " line " \
                      << __LINE__ << ": " << message << std::endl;                       \
            std::terminate();                                                            \
        }                                                                                \
    } while (false)
#else
#define ASSERT(condition, message) \
    do {                           \
    } while (false)
#endif
