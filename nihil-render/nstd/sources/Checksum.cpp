#include "../headers/Checksum.hpp"
#include <string>
#include <vector>
#include <filesystem>

uint64_t nihil::nstd::generateChecksum(const std::string path) 
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << path << std::endl;
        return 0;
    }

    uint32_t checksum = 0;
    char buffer;

    while (file.get(buffer)) {
        checksum ^= static_cast<uint32_t>(buffer) + 0x9e3779b9 + (checksum << 6) + (checksum >> 2);
    }

    file.close();

    return checksum;
}

uint64_t nihil::nstd::loadChecksum(const std::string path)
{
    std::ifstream checkFile;

    if (!(std::filesystem::exists(path + ".checksum")))
    {
        std::cerr << "Unable to find the checksum file" << std::endl;
        saveChecksum(path, generateChecksum(path));
        return 0;
    }

    checkFile.open(path + ".checksum", std::ios::binary);

    std::vector<char> binaryData;
    if (checkFile.is_open()) {
        // Determine the size of the file
        checkFile.seekg(0, std::ios::end);
        size_t fileSize = static_cast<size_t>(checkFile.tellg());
        checkFile.seekg(0, std::ios::beg);

        // Create a vector to hold the file content
        binaryData.resize(fileSize);

        // Read the file into the vector
        checkFile.read(binaryData.data(), fileSize);

        // Close the file
        checkFile.close();
    }
    else {
        std::cerr << "Failed to open checksum files" << std::endl;
    }

    uint64_t currentSum = 0;
    if (binaryData.size() >= sizeof(uint64_t))
    {
        uint64_t* data = reinterpret_cast<uint64_t*>(binaryData.data());
        currentSum = *data;
    }

    return currentSum;
}

bool nihil::nstd::checkForChanges(const std::string path)
{
    uint64_t oldSum = loadChecksum(path);

    uint64_t currentSum = generateChecksum(path);

    if (oldSum == currentSum)
    {
        return false;
    }
    else 
    {
        return true;
        saveChecksum(path, currentSum);
    }
}

void nihil::nstd::saveChecksum(const std::string path, uint64_t checksum)
{
    std::vector<char> data;

    data.resize(sizeof(uint64_t));
    memcpy(data.data(), &checksum, sizeof(uint64_t));

    std::ofstream outputFile(path + ".checksum", std::ios::binary);

    if (outputFile.is_open()) {
        outputFile.write(data.data(), data.size());

        outputFile.close();
    }
    else {
        std::cerr << "Unable to open the file for writing." << std::endl;
    }
}