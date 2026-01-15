#pragma once

//? Move to CMakeLists.txt
#define ENABLE_LOGGING
#define ENABLE_WARNINGS
#define ENABLE_COLORS

#ifdef ENABLE_COLORS
    #define RESET "\033[0m"
    #define RED "\033[31m"
    #define GREEN "\033[32m"
    #define YELLOW "\033[93m"
    #define BLUE "\033[34m"
    #define MAGENTA "\033[35m"
    #define CYAN "\033[36m"
    #define WHITE "\033[37m"
#endif

#include <iostream>
#include <format>

#include "WinCompat.hpp"

namespace nihil
{
    class Logger
    {
    public:
        //Never call this function unless you know what you are doing
        static void Init()
        {
            #ifdef WIN32
                forceLinkCompat();
            #endif
        }

        template<typename... Args>
        inline static void Error(std::format_string<Args...> format, Args&&... args)
        {
            #ifdef ENABLE_LOGGING
                Error(std::format(format, std::forward<Args>(args)...));
            #endif
        }
        inline static void Error(std::string_view what)
        {
            std::cout << std::format(RED "[ERROR]" RESET " {}\n", what);
        }

        template<typename... Args>
        [[noreturn]] inline static void Exception(std::format_string<Args...> fmt, Args&&... args)
        {
            Exception(std::format(fmt, std::forward<Args>(args)...));
        }
        [[noreturn]] inline static void Exception(std::string_view message)
        {
            throw std::runtime_error(std::format(RED "[Exception]" RESET " {}", message));
        }

        template<typename... Args>
        inline static void Log(std::format_string<Args...> format, Args&&... args)
        {
            #ifdef ENABLE_LOGGING
                Log(std::format(format, std::forward<Args>(args)...));
            #endif
        }
        inline static void Log(std::string_view what)
        {
            #ifdef ENABLE_LOGGING
                std::cout << std::format(CYAN "[LOG]" RESET " {}\n", what);
            #endif
        }

        template<typename... Args>
        inline static void Warn(std::format_string<Args...> format, Args&&... args)
        {
            #ifdef ENABLE_WARNINGS
                Warn(std::format(format, std::forward<Args>(args)...));
            #endif
        }
        inline static void Warn(std::string_view what)
        {
            #ifdef ENABLE_WARNINGS
                std::cout << std::format(YELLOW "[WARN]" RESET " {}\n", what);
            #endif
        }

        template<typename... Args>
        inline static void Success(std::format_string<Args...> format, Args&&... args)
        {
            #ifdef ENABLE_WARNINGS
            Success(std::format(format, std::forward<Args>(args)...));
            #endif
        }
        inline static void Success(std::string_view what)
        {
            #ifdef ENABLE_LOGGING
                std::cout << std::format(GREEN "[SUCCESS]" RESET " {}\n", what);
            #endif
        }
    };
}