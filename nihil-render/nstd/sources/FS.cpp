#include "../headers/FS.hpp"
std::string nihil::nstd::LoadFile(std::string& path)
{
    // Specify the file path
    std::string filepath = path;

    // Open the file
    std::ifstream file(filepath);

    // Check if the file is open
    if (file.is_open()) {
        // Read the contents of the file into a string
        std::string fileContent((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());

        // Close the file
        file.close();

        // Display the content of the string
        return fileContent;
    }
    else {
        std::cerr << "Unable to open file: " << filepath << std::endl;
        std::abort();
    }
}