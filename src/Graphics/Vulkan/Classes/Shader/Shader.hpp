#pragma once

#include <vulkan/vulkan.hpp>

#include <shaderc/shaderc.hpp>

#include <fstream>
#include <vector>

#include "Logger.hpp"

#include "Classes/Engine/Engine.hpp"

#include "Classes/Asset/Asset.hpp"

namespace nihil::graphics
{
    class Shader : public Asset
    {
        Engine* engine = nullptr;
    public:
        Resource<vk::ShaderModule> shaderModule;

        std::string name;

        Shader(Engine* _engine);

        void LoadFromSource(const std::string& path);
        void LoadFromBinary(const std::string& path);

        static shaderc_shader_kind shaderKindFromPath(const std::string& path);

        static std::vector<uint32_t> compileGLSL(const std::string& path);

        ~Shader();

        inline vk::ShaderModule* _ptr() { return shaderModule.getResP(); };
    };
}