#ifdef _WIN32

#pragma once

#include <windows.h>
#include <iostream>

#undef near
#undef far
#undef small
#undef DELETE

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef GREEN
#define GREEN "\033[32m"
#endif

#ifndef RESET
#define RESET "\033[0m"
#endif

static inline void enableANSI()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

struct InitCompat
{
    InitCompat()
    {
        enableANSI();
        std::cout << GREEN "[SUCCESS] " RESET "Running in WinCompat mode.\n";
    }
};

extern void forceLinkCompat();

#endif