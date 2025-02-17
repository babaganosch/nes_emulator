#ifndef LOGGING_HPP
#define LOGGING_HPP

namespace nes
{

enum LOG_LEVEL
{
    L_INFO,
    L_DEBUG,
    L_WARNING,
    L_ERROR,
    L_SUCCESS
};

void _log(LOG_LEVEL level, const char* buffer, ...);

#ifdef DEBUG 
    #define LOG_D(...) _log(nes::L_DEBUG, __VA_ARGS__)
#else
    #define LOG_D(...)
#endif

#ifdef WARNINGS
    #define LOG_W(...) _log(nes::L_WARNING, __VA_ARGS__)
#else
    #define LOG_W(...)
#endif

#define LOG_I(...) _log(nes::L_INFO, __VA_ARGS__)
#define LOG_E(...) _log(nes::L_ERROR, __VA_ARGS__)
#define LOG_S(...) _log(nes::L_SUCCESS, __VA_ARGS__)

} // nes

#endif /* LOGGING_HPP */