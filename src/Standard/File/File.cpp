#include "Standard/File.hpp"
#include <filesystem>
#include <fstream>
#include <array>

#include "Standard/Logger.hpp"
#include <openssl/sha.h>

using namespace nihil;

std::string File::LoadFileToString(const std::string &path, std::ios_base::openmode mode)
{
    std::string fileContents;
    std::ifstream file(path, mode);

    if (!file) [[unlikely]]
        Logger::Exception("Failed to open file: {}", path);

    fileContents = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

    file.close();

    return fileContents;
}

std::filesystem::file_time_type File::GetTimestamp(const std::string &path)
{
    try
    {
        return std::filesystem::last_write_time(path);
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        Logger::Exception("File: {} Could not be read. {}", e.path1().string(), e.what());
        return {};
    }
}

std::array<std::byte, 32> File::GetFileChecksum(const std::string &path, std::ios_base::openmode mode)
{
    std::array<std::byte, 32> hash;
    SHA256_CTX ctx;

    std::ifstream file(path, mode | std::ios::binary);
    if (!file)
        Logger::Exception("Failed to open file: {}", path);

    SHA256_Init(&ctx);

    std::array<char, 4096> buffer;
    while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0)
    {
        SHA256_Update(&ctx, buffer.data(), file.gcount());
    }

    SHA256_Final(reinterpret_cast<unsigned char*>(hash.data()), &ctx);

    return hash;
}

void File::WriteToFile(const std::string& path, std::byte *data, size_t size, std::ios_base::openmode mode)
{
    std::ofstream file(path, mode);
    if (!file)
        Logger::Exception("Failed to open file: {}", path);

    file.write(reinterpret_cast<const char*>(data), size);
    if (!file)
        Logger::Exception("Failed to write all bytes to file: {}", path);

    file.close();
}