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
		Resource<vk::Sampler> sampler;
		Engine* engine = nullptr;
	public:
		inline vk::Sampler _sampler() { return sampler.getRes(); };

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

		Sampler(AssetUsage assetUsage, Engine* _engine);

		void create();

		vk::DescriptorSetLayoutBinding  getDescriptorSetLayoutBinding(vk::ShaderStageFlagBits shaderStage, uint32_t binding);

		vk::DescriptorImageInfo getDescriptorInfo();
	};
}