#include "logging.hpp"

#include <cstdio>
#include <stdarg.h>

namespace nes
{

void _log(LOG_LEVEL level, const char* buffer, ...)
{
    static char _buffer[2048];
    static va_list va;

    va_start(va, buffer);
    vsnprintf(_buffer, 2048, buffer, va);
    va_end(va);

    switch (level)
    {
        case L_ERROR: {
            fprintf(stderr, "\x1B[0;30;41m[ERROR]\x1B[0;;m %s\n", _buffer);
        } break;
        case L_WARNING: {
            printf("\x1B[0;30;43m[WARN]\x1B[0;;m %s\n", _buffer);
        } break;
        case L_DEBUG: {
            printf("\x1B[0;33;m[DEBUG]\x1B[0;;m %s\n", _buffer);
        } break;
        default: {
            printf("%s\n", _buffer);
        } break;
    }
}


} // nes