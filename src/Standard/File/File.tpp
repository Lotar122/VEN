template<StdVector T>
T nihil::File::LoadFileToVector(const std::string& path, std::ios_base::openmode mode)
{
    std::ifstream file(path, mode | std::ios::ate | std::ios::binary); // Open file at the end

    if (!file) [[unlikely]]
        Logger::Exception("Failed to open file: {}", path);
    
    size_t fileSize = file.tellg();  // Get file size
    T buffer(fileSize / sizeof(uint32_t)); // Allocate buffer

    file.seekg(0); // Go to beginning
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    file.close();

    return buffer;
}