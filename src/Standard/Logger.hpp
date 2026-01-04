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
        template<typename... Args>
        inline static void Error(std::format_string<Args...> format, Args&&... args)
        {
            #ifdef ENABLE_LOGGING
                Error(std::format(format, std::forward<Args>(args)...));
            #endif
        }
        inline static void Error(std::string_view what)
        {
            std::cout << std::format("[ERROR] {}\n", what);
        }

        template<typename... Args>
        [[noreturn]] inline static void Exception(std::format_string<Args...> fmt, Args&&... args)
        {
            Exception(std::format(fmt, std::forward<Args>(args)...));
        }
        [[noreturn]] inline static void Exception(std::string_view message)
        {
            throw std::runtime_error(std::format("[Exception] {}", message));
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
                std::cout << std::format("[LOG] {}\n", what);
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
                std::cout << std::format("[WARN] {}\n", what);
            #endif
        }
    };
}