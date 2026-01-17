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

        //Only use in development or if you know what you are doing.
        void LoadFromSource(const std::string& path);
        //Only use in development or if you know what you are doing.
        void LoadFromBinary(const std::string& path, size_t offset = 0);

        //This should be used in production it loads shaders from source at first run and on changes. But if the shader didn't change it loads it from a binry cache which is fast.
        void Load(const std::string& path);

        static shaderc_shader_kind shaderKindFromPath(const std::string& path);

        static std::vector<uint32_t> compileGLSL(const std::string& path);

        static inline std::string getCacheNameFromPath(const std::string& path)
        {
            std::string copy = path;
            std::erase_if(copy, [](char c) {if(c == '\\' || std::isspace(c) || c == '/' || c == '.') return true; else return false;});
            copy += ".bin";
            return copy;
        }

        ~Shader();

        inline vk::ShaderModule* _ptr() { return shaderModule.getResP(); };
    };
}