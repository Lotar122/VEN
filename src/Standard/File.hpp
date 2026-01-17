#include "Concepts/StdVector.hpp"
#include "Standard/Logger.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>

namespace nihil
{
    /*This is a static class with helper function for working with files*/
    class File
    {
    public:
        static std::string LoadFileToString(const std::string &path, std::ios_base::openmode mode = std::ios::in);
        template<StdVector T>
        static T LoadFileToVector(const std::string &path, std::ios_base::openmode mode = std::ios::in);
        static std::filesystem::file_time_type GetTimestamp(const std::string& path);
        static std::array<std::byte, 32> GetFileChecksum(const std::string &path, std::ios_base::openmode mode = std::ios::in);
        static void WriteToFile(const std::string& path, std::byte* data, size_t size, std::ios_base::openmode mode = std::ios::out);
    };
}

#include "File/File.tpp"