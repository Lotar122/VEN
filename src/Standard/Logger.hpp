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
                std::cerr << "[ERROR] " << std::format(format, std::forward<Args>(args)...) << '\n';
            #endif
        }
        inline static void Error(const std::string& what)
        {
            #ifdef ENABLE_LOGGING
                std::cerr << "[ERROR] " << what << '\n';
            #endif
        }

        template<typename... Args>
        inline static void Exception(std::format_string<Args...> format, Args&&... args)
        {
            throw std::runtime_error(std::string("[Exception]") + std::format(format, std::forward<Args>(args)...));
        }
        inline static void Exception(const std::string& what)
        {
            throw std::runtime_error(std::string("[Exception]") + what);
        }

        template<typename... Args>
        inline static void Log(std::format_string<Args...> format, Args&&... args)
        {
            #ifdef ENABLE_LOGGING
                std::cout << "[LOG] " << std::format(format, std::forward<Args>(args)...) << '\n';
            #endif
        }
        inline static void Log(const std::string& what)
        {
            #ifdef ENABLE_LOGGING
                std::cout << "[LOG] " << what << '\n';
            #endif
        }

        template<typename... Args>
        inline static void Warn(std::format_string<Args...> format, Args&&... args)
        {
            #ifdef ENABLE_WARNINGS
                std::cout << "[WARN] " << std::format(format, std::forward<Args>(args)...) << '\n';
            #endif
        }
        inline static void Warn(const std::string& what)
        {
            #ifdef ENABLE_WARNINGS
                std::cout << "[WARN] " << what << '\n';
            #endif
        }
    };
}