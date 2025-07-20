#include "Shader.hpp"

using namespace nihil::graphics;

/*
    ! Written mainly by ChatGPT, because i was lazy XD
*/
void Shader::LoadFromBinary(const std::string& path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary); // Open file at the end

    if (!file.is_open()) {
        Logger::Exception("Failed to open SPIR-V file: " + path);
    }
    
    size_t fileSize = file.tellg();  // Get file size
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t)); // Allocate buffer

    file.seekg(0); // Go to beginning
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    file.close();

    vk::ShaderModuleCreateInfo createInfo = {};
    createInfo.codeSize = buffer.size() * sizeof(uint32_t); // Size in bytes
    createInfo.pCode = buffer.data(); // Pointer to SPIR-V data

    shaderModule.assignRes(engine->_device().createShaderModule(createInfo), engine->_device());
}

void Shader::LoadFromSource(const std::string& path)
{
    Logger::Exception("It doesn't work for now, precompile your shaders please!");
}

Shader::Shader(Engine* _engine) : Asset(_engine)
{
    assert(_engine != nullptr);

    engine = _engine;
}

Shader::~Shader()
{
    shaderModule.destroy();
}