#pragma once

#include "Classes/Engine/Engine.hpp"
#include "Classes/Resources/Resources.hpp"

namespace nihil::graphics
{
    static uint32_t findMemoryTypeIndex(vk::PhysicalDeviceMemoryProperties memProperties, vk::MemoryRequirements memRequirements, vk::MemoryPropertyFlags memFlags)
    {
        uint32_t memoryTypeIndex = uint32_t(-1);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
        {
            if (
                (memRequirements.memoryTypeBits & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & memFlags)
            )
            {
                memoryTypeIndex = i;
                break;
            }
        }

        return memoryTypeIndex;
    }
    template<typename T, vk::BufferUsageFlagBits usageT>
    class Buffer
    {
        Engine* engine = nullptr;

        size_t size = 0;
        std::vector<T> data;
        vk::BufferUsageFlagBits usage = usageT;

        vk::MemoryRequirements memRequirements;
        vk::MemoryRequirements stagingMemRequirements;
        vk::PhysicalDeviceMemoryProperties memProperties;

        uint32_t memoryTypeIndex = uint32_t(-1), stagingMemoryTypeIndex = uint32_t(-1);

        Resource<vk::Buffer> buffer;
        Resource<vk::Buffer> stagingBuffer;
        vk::DeviceMemory memory;
        vk::DeviceMemory stagingMemory;

        bool onGPU = false;

        bool destroyed = false;
    public:
        inline vk::Buffer _buffer() { return buffer.getRes(); };
        //return the size of the buffer in bytes
        inline size_t _size() const { return size; };
        //returns the size of the buffer in units of its type
        inline size_t _typedSize() const { return data.size(); };

        inline const Engine* _engine() const { return engine; };

        Buffer(const std::vector<T>& _data, Engine* _engine, vk::MemoryPropertyFlags memPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal)
        {
            assert(_data.size() != 0);
            assert(_engine != nullptr);

            engine = _engine;

            data = _data;

            size = data.size() * sizeof(T);

            // Step 1: Create buffer
            vk::BufferCreateInfo bufferCreateInfo(
                {},                      // Flags (default: none)
                size,              // Size of the buffer
                usageT | vk::BufferUsageFlagBits::eTransferDst, // Usage flags
                vk::SharingMode::eExclusive // Sharing mode
            );

            buffer.assignRes(engine->_device().createBuffer(bufferCreateInfo), engine->_device());

            // Step 2: Get memory requirements
            memRequirements = engine->_device().getBufferMemoryRequirements(buffer.getRes());

            vk::BufferCreateInfo stagingBufferCreateInfo{
                {},
                size,
                vk::BufferUsageFlagBits::eTransferSrc,
                vk::SharingMode::eExclusive
            };

            stagingBuffer.assignRes(engine->_device().createBuffer(stagingBufferCreateInfo), engine->_device());

            stagingMemRequirements = engine->_device().getBufferMemoryRequirements(stagingBuffer.getRes());

            memProperties = engine->_physicalDevice().getMemoryProperties();

            memoryTypeIndex = findMemoryTypeIndex(memProperties, memRequirements, memPropertyFlags);
            stagingMemoryTypeIndex = findMemoryTypeIndex(memProperties, stagingMemRequirements, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

            if (memoryTypeIndex == uint32_t(-1)) {
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

            vk::MemoryAllocateInfo stagingAllocInfo(
                memRequirements.size,    // Allocation size
                stagingMemoryTypeIndex          // Memory type index
            );

            memory = engine->_device().allocateMemory(allocInfo);
            stagingMemory = engine->_device().allocateMemory(stagingAllocInfo);

            void* dataRaw = engine->_device().mapMemory(stagingMemory, 0, size);
            T* dataTyped = reinterpret_cast<T*>(dataRaw);
            //dum dum. we are copying bytes. not floats. although i wonder if copying via the correct typed pointer would give a speed benefit.
            std::memcpy(dataTyped, reinterpret_cast<T*>(data.data()), size);
            engine->_device().unmapMemory(stagingMemory);

            // Step 4: Bind buffer to memory
            engine->_device().bindBufferMemory(buffer.getRes(), memory, 0);
            engine->_device().bindBufferMemory(stagingBuffer.getRes(), stagingMemory, 0);

            vk::Result discardResult = engine->_device().waitForFences(engine->_transferFence(), true, UINT64_MAX);
            discardResult = engine->_device().resetFences(1, &engine->_transferFence());

            // Step 5: Copy staging buffer to buffer
            vk::CommandBufferBeginInfo beginInfo{};
            engine->_mainCommandBuffer().begin(beginInfo);

            vk::BufferCopy copyRegion{ 0, 0, size };
            engine->_mainCommandBuffer().copyBuffer(stagingBuffer.getRes(), buffer.getRes(), copyRegion);

            engine->_mainCommandBuffer().end();

            vk::SubmitInfo submitInfo = {};
            submitInfo.waitSemaphoreCount = 0;
            submitInfo.pWaitSemaphores = nullptr;

            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &engine->_mainCommandBuffer();

            discardResult = engine->_transferQueue().submit(1, &submitInfo, engine->_transferFence());
        }

        inline void freeFromGPU()
        {
            if(!onGPU) return;

            onGPU = false;

            Logger::Log("Freeing the buffer.");

            engine->_device().waitIdle();

            engine->_device().freeMemory(memory);
            engine->_device().freeMemory(stagingMemory);
        }

        void update(const std::vector<T>& _data)
        {
            assert(_data.size() == data.size());

            data = _data;

            bool wasOnGPU = onGPU;

            moveToGPU();

            void* dataRaw = engine->_device().mapMemory(stagingMemory, 0, size);
            T* dataTyped = reinterpret_cast<T*>(dataRaw);
            //dum dum. we are copying bytes. not floats. although i wonder if copying via the correct typed pointer would give a speed benefit.
            std::memcpy(dataTyped, reinterpret_cast<T*>(data.data()), size);
            engine->_device().unmapMemory(stagingMemory);

            vk::Result discardResult = engine->_device().waitForFences(engine->_transferFence(), true, UINT64_MAX);
            discardResult = engine->_device().resetFences(1, &engine->_transferFence());

            // Copy staging buffer to buffer
            vk::CommandBufferBeginInfo beginInfo{};
            engine->_mainCommandBuffer().begin(beginInfo);

            vk::BufferCopy copyRegion{ 0, 0, size };
            engine->_mainCommandBuffer().copyBuffer(stagingBuffer.getRes(), buffer.getRes(), copyRegion);

            engine->_mainCommandBuffer().end();

            vk::SubmitInfo submitInfo = {};
            submitInfo.waitSemaphoreCount = 0;
            submitInfo.pWaitSemaphores = nullptr;

            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &engine->_mainCommandBuffer();

            discardResult = engine->_transferQueue().submit(1, &submitInfo, engine->_transferFence());

            if(!wasOnGPU) freeFromGPU();
        }

        void update(const std::vector<T>&& _data)
        {
            assert(_data.size() == data.size());

            data = std::move(_data);

            bool wasOnGPU = onGPU;

            moveToGPU();

            void* dataRaw = engine->_device().mapMemory(stagingMemory, 0, size);
            T* dataTyped = reinterpret_cast<T*>(dataRaw);
            //dum dum. we are copying bytes. not floats. although i wonder if copying via the correct typed pointer would give a speed benefit.
            std::memcpy(dataTyped, reinterpret_cast<T*>(data.data()), size);
            engine->_device().unmapMemory(stagingMemory);

            vk::Result discardResult = engine->_device().waitForFences(engine->_transferFence(), true, UINT64_MAX);
            discardResult = engine->_device().resetFences(1, &engine->_transferFence());

            //Copy staging buffer to buffer
            vk::CommandBufferBeginInfo beginInfo{};
            engine->_mainCommandBuffer().begin(beginInfo);

            vk::BufferCopy copyRegion{ 0, 0, size };
            engine->_mainCommandBuffer().copyBuffer(stagingBuffer.getRes(), buffer.getRes(), copyRegion);

            engine->_mainCommandBuffer().end();

            vk::SubmitInfo submitInfo = {};
            submitInfo.waitSemaphoreCount = 0;
            submitInfo.pWaitSemaphores = nullptr;

            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &engine->_mainCommandBuffer();

            //vk::Result discardResult = engine->_device().resetFences(1, &frame.inFlightFence);
            discardResult = engine->_transferQueue().submit(1, &submitInfo, engine->_transferFence());

            if(!wasOnGPU) freeFromGPU();
        }

        void destroy()
        {
            assert(!destroyed);

            destroyed = true;

            engine->_device().waitIdle();

            freeFromGPU();

            buffer.destroy();
        }

        inline ~Buffer()
        {
            destroy();
        }
    };
}