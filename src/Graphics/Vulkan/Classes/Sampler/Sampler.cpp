#include "Sampler.hpp"

using namespace nihil::graphics;

Sampler::Sampler(AssetUsage assetUsage, Engine* _engine) : Asset(assetUsage, _engine)
{
    assert(_engine != nullptr);

    engine = _engine;
}

void Sampler::create()
{
    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.magFilter = magFilter;
    samplerInfo.minFilter = minFilter;

    samplerInfo.mipmapMode = mipMapMode;
    samplerInfo.minLod = minLod;
    samplerInfo.maxLod = maxLod;
    samplerInfo.mipLodBias = mipLodBias;
    samplerInfo.mipmapMode = mipMapMode;

    samplerInfo.addressModeU = addressModeU;
    samplerInfo.addressModeV = addressModeV;
    samplerInfo.addressModeW = addressModeW;
    samplerInfo.unnormalizedCoordinates = unnnormalizedUVs;

    samplerInfo.anisotropyEnable = anistropyEnable;
    samplerInfo.maxAnisotropy = maxAnistropy;

    samplerInfo.borderColor = borderColor;

    sampler.assignRes(engine->_device().createSampler(samplerInfo), engine->_device());
}

vk::DescriptorSetLayoutBinding  Sampler::getDescriptorSetLayoutBinding(vk::ShaderStageFlagBits shaderStage, uint32_t binding)
{
    vk::DescriptorSetLayoutBinding samplerBinding{};
    samplerBinding.binding = binding;
    samplerBinding.descriptorType = vk::DescriptorType::eSampler;
    samplerBinding.descriptorCount = 1;
    samplerBinding.stageFlags = shaderStage;
    samplerBinding.pImmutableSamplers = nullptr;

    return samplerBinding;
}

vk::DescriptorImageInfo Sampler::getDescriptorInfo()
{
    vk::DescriptorImageInfo samplerInfo{
        sampler,      // vk::Sampler (or {} if not needed)
        {},    // vk::ImageView (or {} if not needed)
    };

    return samplerInfo;
}