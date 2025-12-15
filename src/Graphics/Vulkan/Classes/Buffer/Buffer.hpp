#pragma once

#include "Classes/Engine/Engine.hpp"
#include "Classes/Resources/Resources.hpp"
#include "StorageAndUniformBuffer.hpp"
#include "FindMemoryTypeIndex.hpp"
#include "Classes/Asset/Asset.hpp"

namespace nihil::graphics
{
    template<typename T, auto usageT, auto propertiesT = static_cast<vk::MemoryPropertyFlags::MaskType>(vk::MemoryPropertyFlagBits::eDeviceLocal)>
    class Buffer : public Asset
    {
        Engine* engine = nullptr;
        vk::Fence transferFence = nullptr;

        size_t size = 0;
        std::vector<T> data;
        static constexpr vk::BufferUsageFlags usage = static_cast<vk::BufferUsageFlags>(usageT);
        static constexpr vk::MemoryPropertyFlags properties = static_cast<vk::MemoryPropertyFlags>(propertiesT);

        vk::MemoryRequirements memRequirements;
        vk::MemoryRequirements stagingMemRequirements;
        vk::PhysicalDeviceMemoryProperties memProperties;

        uint32_t memoryTypeIndex = uint32_t(-1), stagingMemoryTypeIndex = uint32_t(-1);

        Resource<vk::Buffer> buffer;
        Resource<vk::Buffer> stagingBuffer;
        vk::DeviceMemory memory;
        vk::DeviceMemory stagingMemory;

        bool onGPU = false;

        static constexpr bool CPUAccessible = 
            (properties & (vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible)) == (vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);

        bool destroyed = false;
    public:
        inline vk::Buffer _buffer() { return buffer.getRes(); };
        //return the size of the buffer in bytes
        inline size_t _size() const { return size; };
        //returns the size of the buffer in units of its type
        inline size_t _typedSize() const { return data.size(); };

        inline const Engine* _engine() const { return engine; };

        static void copyBuffer(vk::Buffer src, vk::Buffer dst, size_t size, Engine* engine)
        {
            vk::Result discardResult = engine->_device().waitForFences(engine->_transferFence(), true, UINT64_MAX);
            discardResult = engine->_device().resetFences(1, &engine->_transferFence());

            // Step 5: Copy staging buffer to buffer
            vk::CommandBufferBeginInfo beginInfo{};
            engine->_mainCommandBuffer().begin(beginInfo);

            vk::BufferCopy copyRegion{ 0, 0, size };
            engine->_mainCommandBuffer().copyBuffer(src, dst, copyRegion);

            engine->_mainCommandBuffer().end();

            vk::SubmitInfo submitInfo = {};
            submitInfo.waitSemaphoreCount = 0;
            submitInfo.pWaitSemaphores = nullptr;

            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &engine->_mainCommandBuffer();

            discardResult = engine->_transferQueue().submit(1, &submitInfo, engine->_transferFence());
        }

        vk::DescriptorSetLayoutBinding  getDescriptorSetLayoutBinding(vk::ShaderStageFlagBits shaderStage, uint32_t binding)
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
                Logger::Exception("Cannot bind buffer of type: {} via a descriptor set.", usageT);
            }
        }

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
                Logger::Exception("Cannot bind buffer of type: {} via a descriptor set.", usageT);
            }
        }

        Buffer(const std::vector<T>& _data, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            assert(_data.size() != 0);
            assert(_engine != nullptr);
            assert(_engine->_transferFence() != nullptr);

            engine = _engine;
            transferFence = engine->_transferFence();

            data = _data;

            size = data.size() * sizeof(T);

            memProperties = engine->_physicalDevice().getMemoryProperties();

            vk::BufferCreateInfo bufferCreateInfo(
                {},                      // Flags (default: none)
                size,              // Size of the buffer
                usageT | vk::BufferUsageFlagBits::eTransferDst, // Usage flags
                vk::SharingMode::eExclusive // Sharing mode
            );

            buffer.assignRes(engine->_device().createBuffer(bufferCreateInfo), engine->_device());

            memRequirements = engine->_device().getBufferMemoryRequirements(buffer.getRes());

            memoryTypeIndex = findMemoryTypeIndex(memProperties, memRequirements, properties);

            if constexpr (!CPUAccessible)
            {
                Logger::Log("Creating a buffer which will use a staging buffer for copy operations.");

                vk::BufferCreateInfo stagingBufferCreateInfo{
                    {},
                    size,
                    vk::BufferUsageFlagBits::eTransferSrc,
                    vk::SharingMode::eExclusive
                };

                stagingBuffer.assignRes(engine->_device().createBuffer(stagingBufferCreateInfo), engine->_device());

                stagingMemRequirements = engine->_device().getBufferMemoryRequirements(stagingBuffer.getRes());

                stagingMemoryTypeIndex = findMemoryTypeIndex(memProperties, stagingMemRequirements, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

                if (stagingMemoryTypeIndex == uint32_t(-1))
                {
                    Logger::Exception("Failed to find the memory type to create a staging buffer");
                }
            }

            if (memoryTypeIndex == uint32_t(-1)) 
            {
                Logger::Exception("Failed to find suitable memory type to create buffer");
            }     
        }

        Buffer(const std::vector<T>&& _data, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            assert(_data.size() != 0);
            assert(_engine != nullptr);
            assert(_engine->_transferFence() != nullptr);

            engine = _engine;
            transferFence = engine->_transferFence();

            data = std::move(_data);

            size = data.size() * sizeof(T);

            memProperties = engine->_physicalDevice().getMemoryProperties();

            vk::BufferCreateInfo bufferCreateInfo(
                {},                      // Flags (default: none)
                size,              // Size of the buffer
                usageT | vk::BufferUsageFlagBits::eTransferDst, // Usage flags
                vk::SharingMode::eExclusive // Sharing mode
            );

            buffer.assignRes(engine->_device().createBuffer(bufferCreateInfo), engine->_device());

            memRequirements = engine->_device().getBufferMemoryRequirements(buffer.getRes());

            memoryTypeIndex = findMemoryTypeIndex(memProperties, memRequirements, properties);

            if constexpr (!CPUAccessible)
            {
                Logger::Log("Creating a buffer which will use a staging buffer for copy operations.");

                vk::BufferCreateInfo stagingBufferCreateInfo{
                    {},
                    size,
                    vk::BufferUsageFlagBits::eTransferSrc,
                    vk::SharingMode::eExclusive
                };

                stagingBuffer.assignRes(engine->_device().createBuffer(stagingBufferCreateInfo), engine->_device());

                stagingMemRequirements = engine->_device().getBufferMemoryRequirements(stagingBuffer.getRes());

                stagingMemoryTypeIndex = findMemoryTypeIndex(memProperties, stagingMemRequirements, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

                if (stagingMemoryTypeIndex == uint32_t(-1))
                {
                    Logger::Exception("Failed to find the memory type to create a staging buffer");
                }
            }

            if (memoryTypeIndex == uint32_t(-1))
            {
                Logger::Exception("Failed to find suitable memory type to create buffer");
            }
        }

        Buffer(const T* _data, size_t _size, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            assert(_size != 0);
            assert(_data != nullptr);
            assert(_engine != nullptr);
            assert(_engine->_transferFence() != nullptr);

            engine = _engine;
            transferFence = engine->_transferFence();

            size = _size;

            data.resize(size);

            std::memcpy(data.data(), _data, size);

            memProperties = engine->_physicalDevice().getMemoryProperties();

            vk::BufferCreateInfo bufferCreateInfo(
                {},                      // Flags (default: none)
                size,              // Size of the buffer
                usageT | vk::BufferUsageFlagBits::eTransferDst, // Usage flags
                vk::SharingMode::eExclusive // Sharing mode
            );

            buffer.assignRes(engine->_device().createBuffer(bufferCreateInfo), engine->_device());

            memRequirements = engine->_device().getBufferMemoryRequirements(buffer.getRes());

            memoryTypeIndex = findMemoryTypeIndex(memProperties, memRequirements, properties);

            if constexpr (!CPUAccessible)
            {
                Logger::Log("Creating a buffer which will use a staging buffer for copy operations.");

                vk::BufferCreateInfo stagingBufferCreateInfo{
                    {},
                    size,
                    vk::BufferUsageFlagBits::eTransferSrc,
                    vk::SharingMode::eExclusive
                };

                stagingBuffer.assignRes(engine->_device().createBuffer(stagingBufferCreateInfo), engine->_device());

                stagingMemRequirements = engine->_device().getBufferMemoryRequirements(stagingBuffer.getRes());

                stagingMemoryTypeIndex = findMemoryTypeIndex(memProperties, stagingMemRequirements, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

                if (stagingMemoryTypeIndex == uint32_t(-1))
                {
                    Logger::Exception("Failed to find the memory type to create a staging buffer");
                }
            }

            if (memoryTypeIndex == uint32_t(-1))
            {
                Logger::Exception("Failed to find suitable memory type to create buffer");
            }
        }

        void moveToGPU()
        {
            if(onGPU) return;

            onGPU = true;

            Logger::Log("Allocating the buffer.");

            engine->_device().waitIdle();
            
            vk::MemoryAllocateInfo allocInfo(
                memRequirements.size,    // Allocation size
                memoryTypeIndex          // Memory type index
            );

            memory = engine->_device().allocateMemory(allocInfo);

            engine->_device().bindBufferMemory(buffer.getRes(), memory, 0);

            if constexpr (!CPUAccessible)
            {
                Logger::Log("Using a staging buffer");

                vk::MemoryAllocateInfo stagingAllocInfo(
                    memRequirements.size,    // Allocation size
                    stagingMemoryTypeIndex          // Memory type index
                );

                stagingMemory = engine->_device().allocateMemory(stagingAllocInfo);

                engine->_device().bindBufferMemory(stagingBuffer.getRes(), stagingMemory, 0);

                void* dataRaw = engine->_device().mapMemory(stagingMemory, 0, size);
                T* dataTyped = reinterpret_cast<T*>(dataRaw);
                std::memcpy(dataTyped, reinterpret_cast<T*>(data.data()), size);
                engine->_device().unmapMemory(stagingMemory);

                copyBuffer(stagingBuffer.getRes(), buffer.getRes(), size, engine);
            }
            else
            {
                Logger::Log("Using a direct copy");

                void* dataRaw = engine->_device().mapMemory(memory, 0, size);
                T* dataTyped = reinterpret_cast<T*>(dataRaw);
                std::memcpy(dataTyped, reinterpret_cast<T*>(data.data()), size);
                engine->_device().unmapMemory(memory);
            }
        }

        inline void freeFromGPU()
        {
            if(!onGPU) return;

            onGPU = false;

            Logger::Log("Freeing the buffer.");

            engine->_device().waitIdle();

            engine->_device().freeMemory(memory);
            if constexpr (!CPUAccessible) engine->_device().freeMemory(stagingMemory);
        }

        void update(const std::vector<T>& _data)
        {
            assert(_data.size() == data.size());

            data = _data;

            bool wasOnGPU = onGPU;

            moveToGPU();

            if constexpr (!CPUAccessible)
            {
                Logger::Log("Using a staging buffer");

                void* dataRaw = engine->_device().mapMemory(stagingMemory, 0, size);
                T* dataTyped = reinterpret_cast<T*>(dataRaw);
                std::memcpy(dataTyped, reinterpret_cast<T*>(data.data()), size);
                engine->_device().unmapMemory(stagingMemory);

                copyBuffer(stagingBuffer.getRes(), buffer.getRes(), size, engine);
            }
            else
            {
                Logger::Log("Using a direct copy");

                void* dataRaw = engine->_device().mapMemory(memory, 0, size);
                T* dataTyped = reinterpret_cast<T*>(dataRaw);
                std::memcpy(dataTyped, reinterpret_cast<T*>(data.data()), size);
                engine->_device().unmapMemory(memory);
            }

            if(!wasOnGPU) freeFromGPU();
        }

        void update(const std::vector<T>&& _data)
        {
            assert(_data.size() == data.size());

            data = std::move(_data);

            bool wasOnGPU = onGPU;

            moveToGPU();

            if constexpr (!CPUAccessible)
            {
                Logger::Log("Using a staging buffer");

                void* dataRaw = engine->_device().mapMemory(stagingMemory, 0, size);
                T* dataTyped = reinterpret_cast<T*>(dataRaw);
                std::memcpy(dataTyped, reinterpret_cast<T*>(data.data()), size);
                engine->_device().unmapMemory(stagingMemory);

                copyBuffer(stagingBuffer.getRes(), buffer.getRes(), size, engine);
            }
            else
            {
                Logger::Log("Using a direct copy");

                void* dataRaw = engine->_device().mapMemory(memory, 0, size);
                T* dataTyped = reinterpret_cast<T*>(dataRaw);
                std::memcpy(dataTyped, reinterpret_cast<T*>(data.data()), size);
                engine->_device().unmapMemory(memory);
            }

            if(!wasOnGPU) freeFromGPU();
        }

        void update(const T* _data, size_t _size)
        {
            assert(data.size() == _size);

            std::memcpy(data.data(), _data, data.size());

            bool wasOnGPU = onGPU;

            moveToGPU();

            if constexpr (!CPUAccessible)
            {
                Logger::Log("Using a staging buffer");

                void* dataRaw = engine->_device().mapMemory(stagingMemory, 0, size);
                T* dataTyped = reinterpret_cast<T*>(dataRaw);
                std::memcpy(dataTyped, reinterpret_cast<T*>(data.data()), size);
                engine->_device().unmapMemory(stagingMemory);

                copyBuffer(stagingBuffer.getRes(), buffer.getRes(), size, engine);
            }
            else
            {
                Logger::Log("Using a direct copy");

                void* dataRaw = engine->_device().mapMemory(memory, 0, size);
                T* dataTyped = reinterpret_cast<T*>(dataRaw);
                std::memcpy(dataTyped, reinterpret_cast<T*>(data.data()), size);
                engine->_device().unmapMemory(memory);
            }

            if (!wasOnGPU) freeFromGPU();
        }

        void destroy()
        {
            assert(!destroyed);

            destroyed = true;

            engine->_device().waitIdle();

            freeFromGPU();

            buffer.destroy();
            if constexpr (!CPUAccessible)
            {
                stagingBuffer.destroy();
            }
        }

        inline ~Buffer()
        {
            destroy();
        }
    };
}