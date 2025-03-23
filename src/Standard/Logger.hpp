#pragma once

//? Moved to CMakeLists.txt
// #define ENABLE_LOGGING

#include <iostream>

namespace nihil
{
    class Logger
    {
    public:
        inline static void Exception(std::string what)
        {
            throw std::runtime_error(what);
        }

        inline static void Log(std::string what)
        {
            #ifdef ENABLE_LOGGING
                std::cout<<what<<'\n';
            #endif
        }

        inline static void Error(std::string what)
        {
            std::cerr<<what<<'\n';
        }
        inline static void Warn(std::string what)
        {
            std::cout<<what<<'\n';
        }
    };
}