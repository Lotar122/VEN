#pragma once

#include "Classes/Resource/Resource.hpp"
#include "vulkan/vulkan.hpp"
#include "Classes/Resources/Resources.hpp"
#include "Standard/Logger.hpp"
#include "Classes/Texture/Texture.hpp"
#include "Classes/Sampler/Sampler.hpp"
#include "Classes/AssetUsage/AssetUsage.hpp"
#include <unordered_map>

namespace nihil::graphics
{
	class DescriptorAllocator;
    struct DescriptorInfo
	{
		enum class Type
		{
			DescriptorBufferInfo,
			DescriptorImageInfo,
			BufferViewInfo
		};

		union Data
		{
			vk::DescriptorBufferInfo bufferInfo;
			vk::DescriptorImageInfo imageInfo;
			vk::BufferView bufferView;

			Data(vk::DescriptorBufferInfo _bufferInfo) : bufferInfo(_bufferInfo) {}
			Data(vk::DescriptorImageInfo _imageInfo) : imageInfo(_imageInfo) {}
			Data(vk::BufferView _bufferView) : bufferView(_bufferView) {}
			Data() {};
		};

		Data data;
		Type type;

		template<typename T>
		requires requires(T t) { t.getDescriptorInfo(); }
		DescriptorInfo(T* resourcePointer)
		{
			if constexpr (std::is_same_v<T, Texture>) type = Type::DescriptorImageInfo;
			else if constexpr (std::is_same_v<T, Sampler>) type = Type::DescriptorImageInfo;
			else type = Type::DescriptorBufferInfo;

			data = resourcePointer->getDescriptorInfo();
		}

		inline vk::DescriptorBufferInfo& _getBufferInfo() { return data.bufferInfo; };
		inline vk::DescriptorImageInfo& _getImageInfo() { return data.imageInfo; };
		inline vk::BufferView& _getBufferView() { return data.bufferView; };
	};

	struct DescriptorSetLayoutBinding
	{
		vk::DescriptorSetLayoutBinding layoutBinding;
		AssetUsage usage;
		DescriptorInfo descriptorInfo;

		template<typename T>
		requires requires(T t) { t.getDescriptorSetLayoutBinding(vk::ShaderStageFlagBits::eAll, 0); }
		DescriptorSetLayoutBinding(T* resourcePointer, vk::ShaderStageFlagBits shaderStage, uint32_t binding) :
			layoutBinding(resourcePointer->getDescriptorSetLayoutBinding(shaderStage, binding)), usage(resourcePointer->_getAssetUsage()), descriptorInfo(resourcePointer) {};
	};

	template<AssetUsage usageT>
    class DescriptorSetAbstract
    {
    public:
        Engine* engine = nullptr;
		DescriptorAllocator* descriptorAllocator = nullptr;
		AssetUsage usage = usageT;
        
		Resource<vk::DescriptorSetLayout> layout;
		Resource<vk::DescriptorSet> set;

		std::unordered_map<uint32_t, DescriptorSetLayoutBinding> descriptors;

		virtual void create(std::vector<DescriptorSetLayoutBinding>&& _descriptorSetLayoutBindings, DescriptorAllocator* _descriptorAllocator, Engine* _engine) = 0;
    };

	template<AssetUsage usageT>
	class DescriptorSet : public DescriptorSetAbstract<usageT> { };

	template<>
	class DescriptorSet<AssetUsage::Static> : public DescriptorSetAbstract<AssetUsage::Static>
	{
	public:
		virtual void create(std::vector<DescriptorSetLayoutBinding>&& _descriptorSetLayoutBindings, DescriptorAllocator* _descriptorAllocator, Engine* _engine) final override;
	};
}