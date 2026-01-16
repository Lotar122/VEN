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

    std::ifstream file(path);
    if (!file)
        Logger::Exception("Failed to open shader file: {}", path);

    std::string source = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

    auto result = compiler.CompileGlslToSpv(
        source, shaderKindFromPath(path), path.c_str(), options);

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

Shader::Shader(Engine* _engine) : Asset(AssetUsage::Undefined, _engine)
{
    assert(_engine != nullptr);

    engine = _engine;
}

Shader::~Shader()
{
    shaderModule.destroy();
}