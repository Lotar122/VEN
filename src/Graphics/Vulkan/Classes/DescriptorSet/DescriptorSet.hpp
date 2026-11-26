#pragma once

#include "Classes/Resource/Resource.hpp"
#include "vulkan/vulkan.hpp"
#include "Classes/Resources/Resources.hpp"
#include "Classes/Logger/Logger.hpp"
#include "Classes/Texture/Texture.hpp"
#include "Classes/Sampler/Sampler.hpp"
#include "Classes/AssetUsage/AssetUsage.hpp"
#include <type_traits>
#include <unordered_map>

#include <concepts>

template<typename T, auto Member, typename Return>
concept IsMemberFunctionReturnType = std::same_as<Return, std::invoke_result_t<decltype(Member), T&>>;

namespace nihil::graphics
{
	class DescriptorAllocator;
	///The polymorhic type for descriptor information.
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

		///The constructor for DescriptorInfo
		//
		///@tparam T The type of the resource, it has to be either a Buffer, Texture or Sampler type.
		//FUTURE (T): provide a getDescriptorInfo method that returns a descriptor info
		///@param resourcePointer The pointer to the resource
		template<typename T>
		requires requires(T t) { t.getDescriptorInfo(); }
		DescriptorInfo(T* resourcePointer)
		{
			if constexpr (IsMemberFunctionReturnType<T, &T::getDescriptorInfo, vk::DescriptorImageInfo>) type = Type::DescriptorImageInfo;
			else if constexpr (IsMemberFunctionReturnType<T, &T::getDescriptorInfo, vk::BufferView>) type = Type::BufferViewInfo;
			else if constexpr (IsMemberFunctionReturnType<T, &T::getDescriptorInfo, vk::DescriptorBufferInfo>) type = Type::DescriptorBufferInfo;
			else static_assert(false, "Function with an invalid .getDescriptorInfo return type passed to DescriptorInfo");

			data = resourcePointer->getDescriptorInfo();
		}

		///The getter for the buffer info
		inline vk::DescriptorBufferInfo& _getBufferInfo() { return data.bufferInfo; };
		///The getter for the image info
		inline vk::DescriptorImageInfo& _getImageInfo() { return data.imageInfo; };
		///The getter for the buffer view
		inline vk::BufferView& _getBufferView() { return data.bufferView; };
	};

	///The representation for a descriptor set binding
	struct DescriptorSetLayoutBinding
	{
		vk::DescriptorSetLayoutBinding layoutBinding;
		AssetUsage usage;
		DescriptorInfo descriptorInfo;

		///The constructor for a descriptor set layout binding
		//
		///@tparam The type of resource, it has to be either Buffer, Texture or Sampler type
		//
		///@param resourcePointer The pointer to the resource
		///@param shaderStage The shader stages in which this resource must be available
		///@param binding The binding index for the decriptors set
		template<typename T>
		requires requires(T t) { t.getDescriptorSetLayoutBinding(vk::ShaderStageFlagBits::eAll, 0); }
		DescriptorSetLayoutBinding(T* resourcePointer, vk::ShaderStageFlags shaderStage, uint32_t binding) :
			layoutBinding(resourcePointer->getDescriptorSetLayoutBinding(shaderStage, binding)), usage(resourcePointer->_getAssetUsage()), descriptorInfo(resourcePointer) {};
	};

	///The abstract class for a descriptor set
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

		///The pure virtual function for creating a descriptor set.
		virtual void create(std::vector<DescriptorSetLayoutBinding>&& _descriptorSetLayoutBindings, DescriptorAllocator* _descriptorAllocator, Engine* _engine) = 0;
    };

	///A descriptor set representation
	template<AssetUsage usageT>
	class DescriptorSet : public DescriptorSetAbstract<usageT> { };

	///A static descriptor set representation
	template<>
	class DescriptorSet<AssetUsage::Static> : public DescriptorSetAbstract<AssetUsage::Static>
	{
	public:
		///Creates the actuall descriptor set, and writes immediately since this is static data.
		virtual void create(std::vector<DescriptorSetLayoutBinding>&& _descriptorSetLayoutBindings, DescriptorAllocator* _descriptorAllocator, Engine* _engine) final override;
	};
}