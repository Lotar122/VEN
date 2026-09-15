#ifdef _WIN32

#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <iostream>

#undef near
#undef far
#undef small
#undef DELETE

#ifndef GREEN
#define GREEN "\033[32m"
#endif

#ifndef RESET
#define RESET "\033[0m"
#endif

///The function to enable ANSI support on Windows
static inline void enableANSI()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

///The struct that initializes the compatibility for Windows
struct InitCompat
{
    ///The compatibility function
    InitCompat()
    {
        enableANSI();
        std::cout << GREEN "[SUCCESS] " RESET "Running in WinCompat mode.\n";
    }
};

///This is here to force linkage
extern void forceLinkCompat();

#endif