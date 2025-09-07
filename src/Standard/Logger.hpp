#pragma once

//? Move to CMakeLists.txt
#define ENABLE_LOGGING
#define ENABLE_WARNINGS

#include <iostream>
#include <format>

namespace nihil
{
    class Logger
    {
    public:
        inline static void Error(std::format_string<> format, auto&&... args)
        {
            #ifdef ENABLE_LOGGING
                std::cerr << "[ERROR] " << std::vformat(format.get(), std::make_format_args(args...)) << '\n';
            #endif
        }
        inline static void Error(std::string what)
        {
            #ifdef ENABLE_LOGGING
                std::cerr << "[ERROR] " << what << '\n';
            #endif
        }

        inline static void Exception(std::format_string<> format, auto&&... args)
        {
            throw std::runtime_error(std::string("[Exception]") + std::vformat(format.get(), std::make_format_args(args...)));
        }
        inline static void Exception(std::string what)
        {
            throw std::runtime_error(std::string("[Exception]") + what);
        }

        inline static void Log(std::format_string<> format, auto&&... args)
        {
            #ifdef ENABLE_LOGGING
                std::cout << "[LOG] " << std::vformat(format.get(), std::make_format_args(args...)) << '\n';
            #endif
        }
        inline static void Log(std::string what)
        {
            #ifdef ENABLE_LOGGING
                std::cout << "[LOG] " << what << '\n';
            #endif
        }

        inline static void Warn(std::format_string<> format, auto&&... args)
        {
            #ifdef ENABLE_WARNINGS
                std::cout << "[WARN] " << std::vformat(format.get(), std::make_format_args(args...)) << '\n';
            #endif
        }
        inline static void Warn(std::string what)
        {
            #ifdef ENABLE_WARNINGS
                std::cout << "[WARN] " << what << '\n';
            #endif
        }
    };
}