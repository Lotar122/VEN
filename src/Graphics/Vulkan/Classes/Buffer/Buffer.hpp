#pragma once

#include "Macros/break_assert.hpp"

#include "Classes/Engine/Engine.hpp"
#include "Classes/Resources/Resources.hpp"
#include "Concepts/HasFlag.hpp"
#include "Concepts/StdVector.hpp"
#include "StorageBuffer.hpp"
#include "FindMemoryTypeIndex.hpp"
#include "Classes/Asset/Asset.hpp"
#include "vulkan/vulkan.hpp"
#include <iterator>
#include <vector>

namespace nihil::graphics
{
	class Model;

	///The Buffer specialization for std::vector data types T = std::vector<U>
    //
    ///@tparam T The type of the data, here it must be std::vector<U>
    ///@tparam usageT The usage of the buffer, passed as a compile time constant
    ///@tparam propertiesT The memory properties, passed as a compile time constant
	template<typename T, auto usageT, auto propertiesT = static_cast<vk::MemoryPropertyFlags::MaskType>(vk::MemoryPropertyFlagBits::eDeviceLocal)>
	requires StdVector<T>
	class Buffer : public Asset
	{
        ///The std::vector value type
        using U = typename T::value_type;

		friend class Model;
		Engine* engine = nullptr;
		vk::Fence transferFence = nullptr;

		size_t size = 0;
		T data;
		static constexpr vk::BufferUsageFlags usage = static_cast<vk::BufferUsageFlags>(usageT);
		static constexpr vk::MemoryPropertyFlags properties = static_cast<vk::MemoryPropertyFlags>(propertiesT);

        //bool destroyed = false, onGPU = false, allocatedOnGPU = false, bufferCreated = false, directWrite = false;
        uint8_t state = 0x00;

        inline bool destroyed() { return state & 1u << 0; };
        inline bool onGPU() { return state & 1u << 1; };
        inline bool allocatedOnGPU() { return state & 1u << 2; };
        inline bool bufferCreated() { return state & 1u << 3; };
        inline bool directWrite() { return state & 1u << 4; };
        
        //Setters
        inline void setDestroyed(const bool& val) { state = val ? state | (1u << 0) : state & ~(1u << 0); };
        inline void setOnGPU(const bool& val) { state = val ? state | (1u << 1) : state & ~(1u << 1); };
        inline void setAllocatedOnGPU(const bool& val) { state = val ? state | (1u << 2) : state & ~(1u << 2); };
        inline void setBufferCreated(const bool& val) { state = val ? state | (1u << 3) : state & ~(1u << 3); };
        inline void setDirectWrite(const bool& val) { state = val ? state | (1u << 4) : state & ~(1u << 4); };

		vk::MemoryRequirements memRequirements;
		vk::MemoryRequirements stagingMemRequirements;
		vk::PhysicalDeviceMemoryProperties memProperties;

        vk::BufferCreateInfo bufferCreateInfo;
        vk::BufferCreateInfo stagingBufferCreateInfo;

		uint32_t memoryTypeIndex = uint32_t(-1), stagingMemoryTypeIndex = uint32_t(-1);

		vk::Buffer buffer = nullptr;
		vk::Buffer stagingBuffer = nullptr;
		vk::DeviceMemory memory;
		vk::DeviceMemory stagingMemory;

		static constexpr bool CPUAccessible =
			(properties & (vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible)) == (vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);

        enum class UpdateMode
        {
            Immediate,
            Recording,
            Direct
        };

        struct UpdateCommand
        {
            uint64_t srcOffset;
            uint64_t dstOffset;
            uint64_t updateSize;
            uint64_t _pad = 0;
        };

        struct UpdateOptimizer
        {
            std::vector<UpdateCommand> commands;
        };

        struct DirectWriteOptimizer
        {
            std::vector<UpdateCommand> commands;
        };

        UpdateOptimizer updateOptimizer = {};
        DirectWriteOptimizer directWriteOptimizer = {};
        UpdateMode updateMode = UpdateMode::Immediate;
        UpdateMode preRecordingUpdateMode = updateMode;

        void* directDataRaw = nullptr;

        template<UpdateMode updateModeT>
        static void copyBufferImpl(vk::Buffer src, vk::Buffer dst, size_t size, vk::BufferCopy copyRegion, Engine* engine)
        {
            break_assert(dst != nullptr && src != nullptr);

            if constexpr (updateModeT == UpdateMode::Immediate)
            {
                std::ignore = engine->_device().waitForFences(engine->_transferFence(), true, UINT64_MAX);
                std::ignore = engine->_device().resetFences(1, &engine->_transferFence());

                vk::CommandBufferBeginInfo beginInfo{};
                engine->_mainCommandBuffer().begin(beginInfo);
            }

            engine->_mainCommandBuffer().copyBuffer(src, dst, copyRegion);

            if constexpr (updateModeT == UpdateMode::Immediate)
            {
                vk::BufferMemoryBarrier barrier{
                    vk::AccessFlagBits::eTransferWrite,
                    vk::AccessFlagBits::eShaderRead,
                    VK_QUEUE_FAMILY_IGNORED,
                    VK_QUEUE_FAMILY_IGNORED,
                    dst,
                    0,
                    size
                };

                engine->_mainCommandBuffer().pipelineBarrier(
                    vk::PipelineStageFlagBits::eTransfer,
                    vk::PipelineStageFlagBits::eVertexShader,
                    {},
                    nullptr,
                    barrier,
                    nullptr
                );

                engine->_mainCommandBuffer().end();

                vk::SubmitInfo submitInfo = {};
                submitInfo.waitSemaphoreCount = 0;
                submitInfo.pWaitSemaphores = nullptr;

                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &engine->_mainCommandBuffer();

                std::ignore = engine->_transferQueue().submit(1, &submitInfo, engine->_transferFence());
            }
        }

        inline void BufferConsumeImpl(Buffer<T, usageT, propertiesT>* source, size_t newDataSize)
        {
            source->moveToGPU();

            allocateOnGPU();

            copyBuffer(source->buffer, buffer, size, { 0, 0, source->size }, engine);

            updateGPUData({ source->size, source->size, newDataSize * sizeof(U) });

            source->freeFromGPU();
        }

        inline void BufferConstructorImpl(Engine* _engine, AssetUsage _assetUsage)
        {
            break_assert(_engine != nullptr);
            break_assert(_engine->_transferFence() != nullptr);

            engine = _engine;
            transferFence = engine->_transferFence();

            size = data.size() * sizeof(U);

            setBufferCreated(true);

            memProperties = engine->_physicalDevice().getMemoryProperties();

            bufferCreateInfo = vk::BufferCreateInfo{
                {},                      // Flags (default: none)
                size,              // Size of the buffer
                usageT | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc, // Usage flags
                vk::SharingMode::eExclusive // Sharing mode
            };

            buffer = engine->_device().createBuffer(bufferCreateInfo);

            memRequirements = engine->_device().getBufferMemoryRequirements(buffer);

            memoryTypeIndex = findMemoryTypeIndex(memProperties, memRequirements, properties);

            if constexpr (!CPUAccessible)
            {
                Carbo::Logger::Log("Creating a buffer which will use a staging buffer for copy operations.");

                stagingBufferCreateInfo = vk::BufferCreateInfo{
                    {},
                    size,
                    vk::BufferUsageFlagBits::eTransferSrc,
                    vk::SharingMode::eExclusive
                };

                stagingBuffer = engine->_device().createBuffer(stagingBufferCreateInfo);

                stagingMemRequirements = engine->_device().getBufferMemoryRequirements(stagingBuffer);

                stagingMemoryTypeIndex = findMemoryTypeIndex(memProperties, stagingMemRequirements, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

                if (stagingMemoryTypeIndex == uint32_t(-1))
                {
                    Carbo::Logger::Exception("Failed to find the memory type to create a staging buffer");
                }
            }

            if (memoryTypeIndex == uint32_t(-1))
            {
                Carbo::Logger::Exception("Failed to find suitable memory type to create buffer");
            }
        }
    public:
        ///The getter for the vk::Buffer
        inline vk::Buffer _buffer() { return buffer; };
        ///Returns the size of the buffer in bytes
        inline size_t _size() const { return size; };
        ///Returns the size of the buffer in units of its type
        inline size_t _typedSize() const { return data.size(); };

        ///The getter for the underlying data, here std::vector<U>
        inline T& _data() { return data; };

        ///The getter for the engine that owns this buffer
        inline const Engine* _engine() const { return engine; };

        // Disable copy
        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        // Enable move
        Buffer(Buffer&&) noexcept = default;
        Buffer& operator=(Buffer&&) noexcept = default;

        ///The function to copy one buffer to another
        //
        ///@tparam updateModeT Whether the updates are to be accumulated or flushed immediately
        //
        ///@param src The source vk::Buffer
        ///@param dst The destination vk::Buffer
        ///@param size The size of both buffers, must be equal
        ///@param engine The pointer to the Engine owning the buffers
        template<UpdateMode updateModeT = UpdateMode::Immediate>
        static inline void copyBuffer(vk::Buffer src, vk::Buffer dst, size_t size, Engine* engine)
        {
            copyBufferImpl<updateModeT>(src, dst, size, { 0, 0, size }, engine);
        }

        ///The function to a region of one buffer into a region of another buffer
        //
        ///@tparam updateModeT Whether the updates are to be accumulated or flushed immediately
        //
        ///@param src The source vk::Buffer
        ///@param dst The destination vk::Buffer
        ///@param copyRegion The vk::BufferCopy describing the copies effective region
        ///@param size The destination buffers size
        ///@param engine The pointer to the Engine owning the buffers
        template<UpdateMode updateModeT = UpdateMode::Immediate>
        static inline void copyBuffer(vk::Buffer src, vk::Buffer dst, size_t size, vk::BufferCopy copyRegion, Engine* engine)
        {
            copyBufferImpl<updateModeT>(src, dst, size, copyRegion, engine);
        }

        ///Gives you the vk::DescriptorSetLayoutBinding for this buffer
        //
        ///@param shaderStage The flags for the stages the buffer can be bound to
        ///@param binding The binding index in the set that this binding is for
        ///@return The vk::DescriptorSetLayoutBinding object for this buffer
        vk::DescriptorSetLayoutBinding  getDescriptorSetLayoutBinding(vk::ShaderStageFlags shaderStage, uint32_t binding)
        {
            if constexpr (
                usageT == vk::BufferUsageFlagBits::eStorageBuffer ||
                usageT == vk::BufferUsageFlagBits::eUniformBuffer
                )
            {
                vk::DescriptorSetLayoutBinding bufferBinding{};
                bufferBinding.binding = binding;
                /*samplerBinding.descriptorType = vk::DescriptorType::;*/
                if constexpr (usageT == vk::BufferUsageFlagBits::eStorageBuffer) bufferBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
                else if constexpr (usageT == vk::BufferUsageFlagBits::eUniformBuffer) bufferBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
                bufferBinding.descriptorCount = 1;
                bufferBinding.stageFlags = shaderStage;
                bufferBinding.pImmutableSamplers = nullptr;

                return bufferBinding;
            }
            else
            {
                Carbo::Logger::Exception("Cannot bind buffer of type: {} via a descriptor set.", usageT);
            }
        }

        ///Gives you the vk::DescriptorBufferInfo for this buffer
        //
        ///@return The vk::DescriptorBufferInfo for this buffer
        vk::DescriptorBufferInfo getDescriptorInfo()
        {
            if constexpr (
                usageT == vk::BufferUsageFlagBits::eStorageBuffer ||
                usageT == vk::BufferUsageFlagBits::eUniformBuffer
                )
            {
                vk::DescriptorBufferInfo bufferInfo{
                buffer,
                0,
                size
                };

                return bufferInfo;
            }
            else
            {
                Carbo::Logger::Exception("Cannot bind buffer of type: {} via a descriptor set.", usageT);
            }
        }

        ///The constructor that copies the vector
        //
        ///@param _data The constant reference to the data to be copied
        ///@param _engine The Engine owning the buffer
        ///@param _assetUsage The asset usage, don't use this unless it's a StorageBuffer or UniformBuffer
        Buffer(const T& _data, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            break_assert(_data.size() != 0);

            data = _data;

            BufferConstructorImpl(_engine, _assetUsage);
        }

        ///The constructor that moves the data
        //
        ///@param _data The data to be moved
        ///@param _engine The Engine owning the buffer
        ///@param _assetUsage The asset usage, don't use this unless it's a StorageBuffer or UniformBuffer
        Buffer(T&& _data, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            break_assert(_data.size() != 0);

            data = std::move(_data);

            BufferConstructorImpl(_engine, _assetUsage);
        }

        ///The constructor that copies the data from a vector through a pointer
        //
        ///@param _data The pointer to the data that needs to be copied
        ///@param _size The size of the data to be copied
        ///@param _engine The Engine owning the buffer
        ///@param _assetUsage The asset usage, don't use this unless it's a StorageBuffer or UniformBuffer
        Buffer(const U* _data, size_t _size, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            break_assert(_size != 0);
            break_assert(_data != nullptr);

            size = _size * sizeof(U);

            data.resize(_size);

            std::memcpy(data.data(), _data, size);

            BufferConstructorImpl(_engine, _assetUsage);
        }

        ///Consuming constructor (copying new data), the old (smaller) buffer is copied into the new one so that we get the "growing" effect. After this constuctor the buffer will be on GPU. THE OLD BUFFER IS UNUSABLE.
        //
        ///@param source The source buffer that'll be consumed
        ///@param newData The const reference to the newData that'll be copied
        ///@param _engine The Engine owning the buffer
        ///@param _assetUsage The asset usage, don't use this unless it's a StorageBuffer or UniformBuffer
        Buffer(Buffer& source, const T& newData, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            data = std::move(source.data);

            data.reserve(data.size() + newData.size());

            data.insert(data.end(), newData.begin(), newData.end());

            BufferConstructorImpl(_engine, _assetUsage);

            BufferConsumeImpl(&source, newData.size());
        }

        ///Consuming constructor (moving new data), the old (smaller) buffer is copied into the new one so that we get the "growing" effect. After this constuctor the buffer will be on GPU. THE OLD BUFFER IS UNUSABLE.
        //
        ///@param source The source buffer that'll be consumed
        ///@param newData The newData that'll be moved
        ///@param _engine The Engine owning the buffer
        ///@param _assetUsage The asset usage, don't use this unless it's a StorageBuffer or UniformBuffer
        Buffer(Buffer& source, T&& newData, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            data = std::move(source.data);

            data.reserve(data.size() + newData.size());

            data.insert(data.end(), std::make_move_iterator(newData.begin()), std::make_move_iterator(newData.end()));

            BufferConstructorImpl(_engine, _assetUsage);

            BufferConsumeImpl(&source, newData.size());
        }

        ///Consuming constructor (copying new data, through pointer), the old (smaller) buffer is copied into the new one so that we get the "growing" effect. After this constuctor the buffer will be on GPU. THE OLD BUFFER IS UNUSABLE.
        //
        ///@param source The source buffer that'll be consumed
        ///@param newData The pointer to the newData that'll be copied
        ///@param _engine The Engine owning the buffer
        ///@param _assetUsage The asset usage, don't use this unless it's a StorageBuffer or UniformBuffer
        Buffer(Buffer& source, U* newData, size_t newDataSize, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            data = std::move(source.data);

            data.resize(data.size() + newDataSize);

            memcpy(data.data() + source.size, newData, newDataSize * sizeof(U));

            BufferConstructorImpl(_engine, _assetUsage);

            BufferConsumeImpl(&source, newDataSize);
        }

        ///Consuming constructor (copying new data), the old (smaller) buffer is copied into the new one so that we get the "growing" effect. After this constuctor the buffer will be on GPU. THE OLD BUFFER IS UNUSABLE.
        //
        ///@param source The source buffer that'll be consumed
        ///@param newData The const reference to the newData that'll be copied
        ///@param _engine The Engine owning the buffer
        ///@param _assetUsage The asset usage, don't use this unless it's a StorageBuffer or UniformBuffer
        Buffer(Buffer* source, const T& newData, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            data = std::move(source->data);

            data.reserve(data.size() + newData.size());

            data.insert(data.end(), newData.begin(), newData.end());

            //this should work for now.
            BufferConstructorImpl(_engine, _assetUsage);

            BufferConsumeImpl(source, newData.size());
        }

        ///Consuming constructor (moving new data), the old (smaller) buffer is copied into the new one so that we get the "growing" effect. After this constuctor the buffer will be on GPU. THE OLD BUFFER IS UNUSABLE.
        //
        ///@param source The source buffer that'll be consumed
        ///@param newData The newData that'll be moved
        ///@param _engine The Engine owning the buffer
        ///@param _assetUsage The asset usage, don't use this unless it's a StorageBuffer or UniformBuffer
        Buffer(Buffer* source, T&& newData, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            data = std::move(source->data);

            data.reserve(data.size() + newData.size());

            data.insert(data.end(), std::make_move_iterator(newData.begin()), std::make_move_iterator(newData.end()));

            BufferConstructorImpl(_engine, _assetUsage);

            BufferConsumeImpl(source, newData.size());
        }

        ///Consuming constructor (copying new data, pointer to data), the old (smaller) buffer is copied into the new one so that we get the "growing" effect. After this constuctor the buffer will be on GPU. THE OLD BUFFER IS UNUSABLE.
        ///Consuming constructor (copying new data, through pointer), the old (smaller) buffer is copied into the new one so that we get the "growing" effect. After this constuctor the buffer will be on GPU. THE OLD BUFFER IS UNUSABLE.
        //
        ///@param source The source buffer that'll be consumed
        ///@param newData The pointer to the newData that'll be copied
        ///@param _engine The Engine owning the buffer
        ///@param _assetUsage The asset usage, don't use this unless it's a StorageBuffer or UniformBuffer
        Buffer(Buffer* source, U* newData, size_t newDataSize, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            data = std::move(source->data);

            data.resize(data.size() + newDataSize);

            memcpy(data.data() + source->size, newData, newDataSize * sizeof(U));

            BufferConstructorImpl(_engine, _assetUsage);

            BufferConsumeImpl(source, newDataSize);
        }

        ///This function allocates the buffer on GPU, can be called when already on GPU it just ignores it.
        void allocateOnGPU()
        {
            if (allocatedOnGPU()) return;

            Carbo::Logger::Log("Allocating the buffer.");

            engine->_device().waitIdle();

            vk::MemoryAllocateInfo allocInfo(
                memRequirements.size,    // Allocation size
                memoryTypeIndex          // Memory type index
            );

            memory = engine->_device().allocateMemory(allocInfo);
            if(!bufferCreated()) buffer = engine->_device().createBuffer(bufferCreateInfo);

            engine->_device().bindBufferMemory(buffer, memory, 0);

            if constexpr (!CPUAccessible)
            {
                Carbo::Logger::Log("Allocating a staging buffer");

                vk::MemoryAllocateInfo stagingAllocInfo(
                    memRequirements.size,    // Allocation size
                    stagingMemoryTypeIndex          // Memory type index
                );

                stagingMemory = engine->_device().allocateMemory(stagingAllocInfo);
                if (!bufferCreated()) stagingBuffer = engine->_device().createBuffer(stagingBufferCreateInfo);

                engine->_device().bindBufferMemory(stagingBuffer, stagingMemory, 0);
            }

            setAllocatedOnGPU(true);
            setBufferCreated(true);
        }
        
        ///The function for moving the actuall data onto the GPU
        //
        ///@tparam updateModeT Whether the updates are to be accumulated or flushed immediately 
        template<UpdateMode updateModeT = UpdateMode::Immediate>
        void moveToGPU()
        {
            if (onGPU()) return;

            allocateOnGPU();

            Carbo::Logger::Log("Moving the buffer.");

            engine->_device().waitIdle();

            if constexpr (!CPUAccessible)
            {
                void* dataRaw = engine->_device().mapMemory(stagingMemory, 0, size);
                std::memcpy(reinterpret_cast<U*>(dataRaw), reinterpret_cast<U*>(data.data()), size);

                engine->_device().unmapMemory(stagingMemory);

                copyBuffer<updateModeT>(stagingBuffer, buffer, size, engine);
            }
            else
            {
                Carbo::Logger::Log("Using a direct copy");

                void* dataRaw = engine->_device().mapMemory(memory, 0, size);
                std::memcpy(reinterpret_cast<U*>(dataRaw), reinterpret_cast<U*>(data.data()), size);
                engine->_device().unmapMemory(memory);
            }

            setOnGPU(true);
        }

        ///Frees the memory from GPU
        inline void freeFromGPU()
        {
            if (!allocatedOnGPU()) return;

            setOnGPU(false);
            setAllocatedOnGPU(false);
            setBufferCreated(false);

            Carbo::Logger::Log("Freeing the buffer.");

            engine->_device().waitIdle();

            engine->_device().destroyBuffer(buffer);
            engine->_device().freeMemory(memory);
            #if defined(_DEBUG) || !defined(NDEBUG)
                buffer = nullptr;
            #endif
            if constexpr (!CPUAccessible)
            {
                engine->_device().destroyBuffer(stagingBuffer);
                engine->_device().freeMemory(stagingMemory);

                #if defined(_DEBUG) || !defined(NDEBUG)
                    stagingBuffer = nullptr;
                #endif
            }
        }
    private:
        template<UpdateMode updateModeT = UpdateMode::Immediate>
        inline void updateGPUData(vk::BufferCopy updateRegion, const U* _directData = nullptr, void* _dataRaw = nullptr)
        {
            if constexpr (updateModeT == UpdateMode::Direct)
            {
                if (!onGPU()) [[unlikely]] Carbo::Logger::Warn("You might have un-pushed changes CPU side. Upload before direct writing perhaps?");

                setOnGPU(true);

                std::memcpy(
                    reinterpret_cast<U*>(_dataRaw) + updateRegion.dstOffset,
                    reinterpret_cast<const std::byte*>(_directData) + updateRegion.srcOffset,
                    updateRegion.size
                );
            }
            else
            {
                bool wasOnGPU = onGPU();

                moveToGPU<updateModeT>();

                if constexpr (!CPUAccessible)
                {
                    Carbo::Logger::Log("Using a staging buffer");

                    void* dataRaw = engine->_device().mapMemory(stagingMemory, updateRegion.dstOffset, updateRegion.size);
                    std::memcpy(
                        reinterpret_cast<U*>(dataRaw),
                        reinterpret_cast<std::byte*>(data.data()) + updateRegion.srcOffset,
                        updateRegion.size
                    );
                    engine->_device().unmapMemory(stagingMemory);

                    copyBuffer<updateModeT>(stagingBuffer, buffer, size, updateRegion, engine);
                }
                else
                {
                    Carbo::Logger::Log("Using a direct copy");

                    void* dataRaw = engine->_device().mapMemory(memory, updateRegion.dstOffset, updateRegion.size);
                    std::memcpy(
                        reinterpret_cast<U*>(dataRaw),
                        reinterpret_cast<std::byte*>(data.data()) + updateRegion.srcOffset,
                        updateRegion.size
                    );
                    engine->_device().unmapMemory(memory);
                }

                if constexpr (updateModeT == UpdateMode::Immediate) if (!wasOnGPU) freeFromGPU();
            }
        }

        inline void recordUpdate(const vk::BufferCopy& updateRegion)
        {
            if (updateOptimizer.commands.empty()) [[unlikely]]
            {
                updateOptimizer.commands.emplace_back(updateRegion.srcOffset, updateRegion.dstOffset, updateRegion.size);

                return;
            }
            auto& prev = updateOptimizer.commands.back();
            if (prev.dstOffset + prev.updateSize == updateRegion.dstOffset)
            {
                prev.updateSize += updateRegion.size;
            }
            else
            {
                updateOptimizer.commands.emplace_back(updateRegion.srcOffset, updateRegion.dstOffset, updateRegion.size);
            }
        }

        inline void recordDirectWrite(const vk::BufferCopy& updateRegion)
        {
            if (directWriteOptimizer.commands.empty()) [[unlikely]]
            {
                directWriteOptimizer.commands.emplace_back(updateRegion.srcOffset, updateRegion.dstOffset, updateRegion.size);

                return;
            }
            auto& prev = directWriteOptimizer.commands.back();
            if (prev.dstOffset + prev.updateSize == updateRegion.dstOffset)
            {
                prev.updateSize += updateRegion.size;
            }
            else
            {
                directWriteOptimizer.commands.emplace_back(updateRegion.srcOffset, updateRegion.dstOffset, updateRegion.size);
            }
        }
    public:
        ///Used with the direct write optimization on mainly on UMA architectures. This initiates the direct write mode.
        inline void beginDirectWrite()
        {
            preRecordingUpdateMode = updateMode;
            updateMode = UpdateMode::Direct;

            directWriteOptimizer.commands.clear();
            directWriteOptimizer.commands.reserve(128);

            if constexpr (!CPUAccessible)
            {
                directDataRaw = engine->_device().mapMemory(stagingMemory, 0, size);
            }
            else
            {
                directDataRaw = engine->_device().mapMemory(memory, 0, size);
            }
        }

        ///Flushes all of the direct writes to the GPU if not on UMA. This ends the direct write mode.
        void executeDirectWrites()
        {
            updateMode = preRecordingUpdateMode;

            if constexpr (!CPUAccessible)
            {
                engine->_device().unmapMemory(stagingMemory);
            }
            else
            {
                engine->_device().unmapMemory(memory);
            }

            std::ignore = engine->_device().waitForFences(engine->_transferFence(), true, UINT64_MAX);
            std::ignore = engine->_device().resetFences(1, &engine->_transferFence());

            vk::CommandBufferBeginInfo beginInfo{};
            engine->_mainCommandBuffer().begin(beginInfo);

            for (int i = 0; i < directWriteOptimizer.commands.size(); i++)
            {
                //std::cout << "Staging -> DeviceLocal\n";

                //Copy staging to buffer
                //updateGPUData<UpdateMode::Direct>({ updateOptimizer.commands[i].srcOffset, updateOptimizer.commands[i].dstOffset, updateOptimizer.commands[i].updateSize });
                copyBuffer<UpdateMode::Direct>(stagingBuffer, buffer, size, { directWriteOptimizer.commands[i].srcOffset, directWriteOptimizer.commands[i].dstOffset, directWriteOptimizer.commands[i].updateSize }, engine);
            }

            #if defined(_DEBUG) || !defined(NDEBUG)
                directDataRaw = nullptr;
            #endif

            break_assert(buffer != nullptr && stagingBuffer != nullptr);

            vk::BufferMemoryBarrier barrier{
                vk::AccessFlagBits::eTransferWrite,
                vk::AccessFlagBits::eShaderRead,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                buffer,
                0,
                size
            };

            engine->_mainCommandBuffer().pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eVertexShader,
                {},
                nullptr,
                barrier,
                nullptr
            );

            engine->_mainCommandBuffer().end();

            vk::SubmitInfo submitInfo = {};
            submitInfo.waitSemaphoreCount = 0;
            submitInfo.pWaitSemaphores = nullptr;

            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &engine->_mainCommandBuffer();

            std::ignore = engine->_transferQueue().submit(1, &submitInfo, engine->_transferFence());
        }

        ///Sets the global update mode.
        //
        ///@param _updateMode The update mode to be set
        inline void setUpdateMode(UpdateMode _updateMode)
        {
            updateMode = _updateMode;
        }

        ///Initiates the recording update mode.
        inline void beginUpdateRecording()
        {
            preRecordingUpdateMode = updateMode;
            updateMode = UpdateMode::Recording;

            updateOptimizer.commands.clear();
            updateOptimizer.commands.reserve(128);
        }

        ///Flushes the recorded updates onto the GPU.
        void executeRecordedUpdates()
        {
            bool wasOnGPU = onGPU();

            moveToGPU();

            updateMode = preRecordingUpdateMode;

            std::ignore = engine->_device().waitForFences(engine->_transferFence(), true, UINT64_MAX);
            std::ignore = engine->_device().resetFences(1, &engine->_transferFence());

            vk::CommandBufferBeginInfo beginInfo{};
            engine->_mainCommandBuffer().begin(beginInfo);

            for (int i = 0; i < updateOptimizer.commands.size(); i++)
            {
                updateGPUData<UpdateMode::Recording>({ updateOptimizer.commands[i].srcOffset, updateOptimizer.commands[i].dstOffset, updateOptimizer.commands[i].updateSize });
            }

            break_assert(buffer != nullptr && stagingBuffer != nullptr);

            vk::BufferMemoryBarrier barrier{
                vk::AccessFlagBits::eTransferWrite,
                vk::AccessFlagBits::eShaderRead,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                buffer,
                0,
                size
            };

            engine->_mainCommandBuffer().pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eVertexShader,
                {},
                nullptr,
                barrier,
                nullptr
            );

            engine->_mainCommandBuffer().end();

            vk::SubmitInfo submitInfo = {};
            submitInfo.waitSemaphoreCount = 0;
            submitInfo.pWaitSemaphores = nullptr;

            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &engine->_mainCommandBuffer();

            std::ignore = engine->_transferQueue().submit(1, &submitInfo, engine->_transferFence());

            if (!wasOnGPU) freeFromGPU();
        }

        ///Used to update the entire data from a vector, copies
        //
        ///@param _data The data to be copied
        void update(const T& _data)
        {
            break_assert(_data.size() == data.size());

            if (updateMode != UpdateMode::Direct)
            {
                data = _data;

                updateGPUData({ 0, 0, size });
            }
            else
            {
                updateGPUData<UpdateMode::Direct>({ 0, 0, size }, _data.data());
            }
        }

        ///Used to update the entire data from a vector, moves
        //
        ///@param _data The data to be moved
        void update(const T&& _data)
        {
            break_assert(_data.size() == data.size());

            if (updateMode != UpdateMode::Direct)
            {
                data = std::move(_data);

                updateGPUData({ 0, 0, size });
            }
            else
            {
                updateGPUData<UpdateMode::Direct>({ 0, 0, size }, _data.data());
            }
        }

        ///Used to update the entire data from a vector, copies through pointer to data
        //
        ///@param _data The data to be copied
        ///@param _size The size of the data
        void update(const U* _data, size_t _size)
        {
            break_assert(data.size() == _size);

            if (updateMode != UpdateMode::Direct)
            {
                std::memcpy(data.data(), _data, data.size());

                updateGPUData({ 0, 0, size });
            }
            else
            {
                updateGPUData<UpdateMode::Direct>({ 0, 0, size }, _data);
            }
        }

        ///Used to update part of the data from a vector, copies through pointer to data
        //
        ///@param _data The data to be copied
        ///@param updateRegion The region to be updated
        void update(const U* _data, vk::BufferCopy updateRegion)
        {
            break_assert(_data != nullptr);

            if (updateMode != UpdateMode::Direct)
            {
                std::memcpy(
                    reinterpret_cast<std::byte*>(data.data()) + updateRegion.dstOffset,
                    _data + updateRegion.srcOffset,
                    updateRegion.size
                );

                if (updateMode == UpdateMode::Immediate) updateGPUData(updateRegion);
                else recordUpdate(updateRegion);
            }
            else
            {
                allocateOnGPU();

                updateGPUData<UpdateMode::Direct>(updateRegion, _data, directDataRaw);
                recordDirectWrite(updateRegion);
            }
        }

        ///The helper destructor
        void destroy()
        {
            break_assert(!destroyed());

            setDestroyed(true);

            engine->_device().waitIdle();

            freeFromGPU();
        }

        inline ~Buffer()
        {
            destroy();
        }
	};
}