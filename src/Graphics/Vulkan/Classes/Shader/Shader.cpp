#include "Shader.hpp"

#include "Standard/File.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>

using namespace nihil::graphics;
using namespace nihil;

/*
    ! Written mainly by ChatGPT, because i was lazy XD
    This code is trivial anyway so who cares
*/
void Shader::LoadFromBinary(const std::string& path, size_t offset)
{
    std::vector<uint32_t> buffer = std::move(File::LoadFileToVector<std::vector<uint32_t>>(path)); // Allocate buffer

    vk::ShaderModuleCreateInfo createInfo = {};
    createInfo.codeSize = buffer.size() * sizeof(uint32_t) - offset; // Size in bytes
    createInfo.pCode = buffer.data() + offset; // Pointer to SPIR-V data

    shaderModule.assignRes(engine->_device().createShaderModule(createInfo), engine->_device());
}

shaderc_shader_kind Shader::shaderKindFromPath(const std::string& path)
{
    if (path.ends_with(".vert")) return shaderc_vertex_shader;
    if (path.ends_with(".frag")) return shaderc_fragment_shader;
    if (path.ends_with(".comp")) return shaderc_compute_shader;
    Logger::Exception("Unknown shader type.");
}

std::vector<uint32_t> Shader::compileGLSL(const std::string& path)
{
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    options.SetTargetEnvironment(
        shaderc_target_env_vulkan,
        shaderc_env_version_vulkan_1_2);

    options.SetOptimizationLevel(
        shaderc_optimization_level_performance);

    auto result = compiler.CompileGlslToSpv(
        File::LoadFileToString(path), shaderKindFromPath(path), path.c_str(), options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        Logger::Exception(result.GetErrorMessage());
    }

    return { result.begin(), result.end() };
}

void Shader::LoadFromSource(const std::string& path)
{
    std::vector<uint32_t> compiled = compileGLSL(path);

    vk::ShaderModuleCreateInfo createInfo = {};
    createInfo.codeSize = compiled.size() * sizeof(uint32_t); // Size in bytes
    createInfo.pCode = compiled.data(); // Pointer to SPIR-V data

    shaderModule.assignRes(engine->_device().createShaderModule(createInfo), engine->_device());
}

static inline std::vector<uint32_t> compileAndSaveToCache(const std::string& path, const std::string& cacheName, Engine* engine = nullptr)
{
    std::vector<uint32_t> compiledShader = Shader::compileGLSL(path);
    //Create cache entry
    std::filesystem::file_time_type timestamp = File::GetTimestamp(path);
    std::array<std::byte, 32> hash = std::move(File::GetFileChecksum(path));

    std::byte* cacheEntry = (std::byte*)std::malloc(sizeof(std::filesystem::file_time_type) + (sizeof(std::byte) * 32) + (compiledShader.size() * sizeof(uint32_t)));

    std::memcpy(
        cacheEntry, 
        &timestamp, 
        sizeof(std::filesystem::file_time_type)
    );
    std::memcpy(
        cacheEntry + sizeof(std::filesystem::file_time_type), 
        hash.data(), 
        sizeof(std::byte) * 32
    );
    std::memcpy(
        cacheEntry + sizeof(std::filesystem::file_time_type) + (sizeof(std::byte) * 32), 
        compiledShader.data(), 
        compiledShader.size() * sizeof(uint32_t)
    );

    File::WriteToFile(
        engine->_directory() + "/ShaderCache/" + cacheName, 
        cacheEntry, 
        sizeof(std::filesystem::file_time_type) + (sizeof(std::byte) * 32) + (compiledShader.size() * sizeof(uint32_t))
    );

    std::free(cacheEntry);

    return compiledShader;
}

void Shader::Load(const std::string& path)
{
    //Check if the shader is in cache
    std::string cacheName = std::move(getCacheNameFromPath(path));
    std::vector<uint32_t> compiledShader;

    if(!std::filesystem::exists(engine->_directory() + "/ShaderCache/" + cacheName)) [[unlikely]]
    {
        compiledShader = compileAndSaveToCache(path, cacheName, engine);
    }
    else
    {
        std::filesystem::file_time_type sourceTimestamp = File::GetTimestamp(path);
        
        std::ifstream cacheFile(engine->_directory() + "/ShaderCache/" + cacheName, std::ios::binary);
        if (!cacheFile)
            Logger::Exception("Failed to open file: {}", path);

        std::filesystem::file_time_type cacheTimestamp;

        cacheFile.read(reinterpret_cast<char*>(&cacheTimestamp), sizeof(std::filesystem::file_time_type));

        if(sourceTimestamp > cacheTimestamp)
        {
            compiledShader = compileAndSaveToCache(path, cacheName, engine);
        }
        else
        {
            std::array<std::byte, 32> hash;
            cacheFile.read(reinterpret_cast<char*>(hash.data()), sizeof(std::byte) * 32);

            std::array<std::byte, 32> sourceHash = File::GetFileChecksum(path);

            //The vector is empty...
            if(std::memcmp(hash.data(), sourceHash.data(), sizeof(std::byte) * 32) == 0)
            {
                std::streampos currentPos = cacheFile.tellg();
                cacheFile.seekg(0, std::ios::end);
                std::streampos endPos = cacheFile.tellg();
                cacheFile.seekg(currentPos, std::ios::beg);
                size_t shaderBinarySize = static_cast<std::size_t>(endPos - currentPos);
                compiledShader.resize(shaderBinarySize / sizeof(uint32_t));
                cacheFile.read(reinterpret_cast<char*>(compiledShader.data()), shaderBinarySize);
            }
            else
            {
                compiledShader = compileAndSaveToCache(path, cacheName, engine);
            }
        }

        cacheFile.close();
    }

    if (compiledShader.empty()) [[unlikely]]
    {
        Logger::Exception("Shader SPIR-V is empty for path: {}", path);
    }

    vk::ShaderModuleCreateInfo createInfo = {};
    createInfo.codeSize = compiledShader.size() * sizeof(uint32_t); // Size in bytes
    createInfo.pCode = compiledShader.data(); // Pointer to SPIR-V data

    shaderModule.assignRes(engine->_device().createShaderModule(createInfo), engine->_device());
}

Shader::Shader(Engine* _engine) : Asset(AssetUsage::Undefined, _engine)
{
    assert(_engine != nullptr);

    engine = _engine;
}

Shader::~Shader()
{
    shaderModule.destroy();
}