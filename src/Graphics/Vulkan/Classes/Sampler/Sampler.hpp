#pragma once

#include <vulkan/vulkan.hpp>

#include "Classes/Engine/Engine.hpp"
#include "Classes/Resources/Resources.hpp"

#include "Classes/Asset/Asset.hpp"
#include "Classes/AssetUsage/AssetUsage.hpp"


namespace nihil::graphics
{
	class Sampler : public Asset
	{
		Engine* engine = nullptr;
	public:
		vk::Filter magFilter = vk::Filter::eLinear;
		vk::Filter minFilter = vk::Filter::eLinear;
		vk::SamplerAddressMode addressModeU = vk::SamplerAddressMode::eRepeat;
		vk::SamplerAddressMode addressModeV = vk::SamplerAddressMode::eRepeat;
		vk::SamplerAddressMode addressModeW = vk::SamplerAddressMode::eRepeat;

		uint32_t anistropyEnable = VK_FALSE;
		float maxAnistropy = 1.0f;

		vk::BorderColor borderColor = vk::BorderColor::eIntOpaqueBlack;

		uint32_t unnnormalizedUVs = VK_FALSE;

		vk::SamplerMipmapMode mipMapMode = vk::SamplerMipmapMode::eLinear;

		float minLod = 0.0f;
		float maxLod = 0.0f;
		float mipLodBias = 0.0f;

		Resource<vk::Sampler> sampler;

		Sampler(AssetUsage assetUsage, Engine* _engine) : Asset(assetUsage, _engine)
		{
			assert(_engine != nullptr);

			engine = _engine;
		}

		void create()
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

		vk::DescriptorSetLayoutBinding  getDescriptorSetLayoutBinding(vk::ShaderStageFlagBits shaderStage, uint32_t binding)
		{
			vk::DescriptorSetLayoutBinding samplerBinding{};
			samplerBinding.binding = binding;
			samplerBinding.descriptorType = vk::DescriptorType::eSampler;
			samplerBinding.descriptorCount = 1;
			samplerBinding.stageFlags = shaderStage;
			samplerBinding.pImmutableSamplers = nullptr;

			return samplerBinding;
		}

		vk::DescriptorImageInfo getDescriptorInfo()
		{
			vk::DescriptorImageInfo samplerInfo{
				sampler,      // vk::Sampler (or {} if not needed)
				{},    // vk::ImageView (or {} if not needed)
			};

			return samplerInfo;
		}
	};
}