#pragma once

#include <limits>
#include <vector>

#include "Classes/BlockAllocator/BlockAllocator.hpp"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "Classes/ECSAllocator/ECSAllocator.hpp"
#include "Classes/Object/Object.hpp"
#include "Classes/DescriptorAllocator/DescriptorAllocator.hpp"
#include "Classes/Resource/Resource.hpp"
#include "Structs/BVHNode.hpp"

#include "Classes/AtomicBumpAllocator/AtomicBumpAllocator.hpp"

namespace nihil::graphics
{
    class Camera;
    class Engine;
    class Model;

    class Scene
    {
        struct ObjectInstance
        {
            uint64_t key;
            Object* object;
        };

        struct InstanceDataSlot
        {
            size_t prevOffset = std::numeric_limits<size_t>::max();
            size_t lastResident = std::numeric_limits<size_t>::max();
            size_t currentResident = std::numeric_limits<size_t>::max();
            size_t currentResidentRenderIndex = std::numeric_limits<size_t>::max();
            
            uint64_t prevAssignedFrame = std::numeric_limits<uint64_t>::max();
            uint64_t prevWriteFrame = std::numeric_limits<size_t>::max();

            InstanceDataSlot(size_t _currentResident, size_t _currentResidentRenderIndex, uint64_t _prevAssignedFrame) : 
                currentResident(_currentResident), currentResidentRenderIndex(_currentResidentRenderIndex), prevAssignedFrame(_prevAssignedFrame) {};

            InstanceDataSlot() {};
        };

        template<bool _freeList = false, bool _homeless = false>
        static void constructSlots(
            size_t instanceDataSize,
            size_t instanceDataChunkSize,
            ObjectInstance* toRender,
            size_t toRenderSize,
            std::vector<InstanceDataSlot>& slots, 
            size_t thisFrame, 
            Buffer<std::vector<std::byte>, vk::BufferUsageFlagBits::eVertexBuffer>& instanceBuffer, 
            std::vector<size_t>* __freeList = nullptr, std::vector<size_t>* __homeless = nullptr
        );

        Engine* engine = nullptr;
        Carbo::BlockAllocator<Buffer<std::vector<std::byte>, vk::BufferUsageFlagBits::eVertexBuffer>> instanceBufferAllocator;
        Carbo::ECSAllocator<BVH2Node> BVHNodeAllocator;
        std::vector<size_t> BVHIndices;
        std::vector<size_t> toRender;

        // Carbo::ECSAllocator<BVH4Node> BVH4NodeAllocator;
        // Carbo::ECSAllocator<BVH4LeafNode> BVH4LeafNodeAllocator;
        // Carbo::ECSAllocator<BVH4ColdNode> BVH4ColdNodeAllocator;

        Carbo::AtomicBumpAllocator<alignof(BVH4Node)>* BVH4NodeAllocator = nullptr;
        alignas(Carbo::AtomicBumpAllocator<alignof(BVH4Node)>) std::byte BVH4NodeAllocatorMemory[sizeof(BVH4Node)];

        Carbo::ECSAllocator<BVH2Node> BVH2NodeAllocator;

        size_t BVHRoot = std::numeric_limits<size_t>::max();

        //instead of Model* use two asset ids (model, material) packed into a uint64_t
        std::vector<std::pair<uint64_t, Buffer<std::vector<std::byte>, vk::BufferUsageFlagBits::eVertexBuffer>*>> instanceBuffers;
        std::vector<std::pair<uint64_t, std::vector<InstanceDataSlot>>> instanceSlots;

        std::vector<ObjectInstance> instancedDraws;
        std::vector<Object*> normalDraws;

        std::vector<Object*> objects;

        //New instancing system resources
        std::vector<size_t> instanceDataSlotFreeList;
        std::vector<size_t> homelessData;

        static constexpr size_t shadowResolution = 2048;

        //Shadows
        Resource<vk::Image> shadowCubeMap;
        Resource<vk::ImageView> shadowCubeMapView;
        Resource<vk::DeviceMemory> shadowCubeMapMemory;

        std::array<Resource<vk::ImageView>, 6> shadowViews;
        std::array<glm::mat4, 6> shadowViewMatricies;
    public:

        inline void addObject(Object* object) { objects.push_back(object); };
        void addObjects(const Object** newObjects, size_t size);
        void addObjects(Object* newObjects, size_t size);
        void addObjects(const std::vector<Object*>& newObjects);
        void addObjects(std::vector<Object>& newObjects);

        inline void use() { for (Object* o : objects) { o->use(); } };
        inline void unuse() { for (Object* o : objects) { o->unuse(); } };

        void lightingPass(vk::CommandBuffer& commandBuffer, Camera* camera, Pipeline* debugPipeline, DescriptorAllocator* descriptorAllocator);
        void recordCommands(vk::CommandBuffer& commandBuffer, Camera* camera, Pipeline* debugPipeline, DescriptorAllocator* descriptorAllocator = nullptr);

        Scene(Engine* _engine)
        {
            assert(_engine != nullptr);

            engine = _engine;

            vk::ImageCreateInfo shadowCubeMapInfo{};
            shadowCubeMapInfo.imageType = vk::ImageType::e2D;
            shadowCubeMapInfo.format = vk::Format::eD32Sfloat;
            shadowCubeMapInfo.extent = vk::Extent3D{shadowResolution, shadowResolution, 1};
            shadowCubeMapInfo.mipLevels = 1;
            shadowCubeMapInfo.arrayLayers = 6;
            shadowCubeMapInfo.samples = vk::SampleCountFlagBits::e1;
            shadowCubeMapInfo.tiling = vk::ImageTiling::eOptimal;
            shadowCubeMapInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
            shadowCubeMapInfo.flags = vk::ImageCreateFlagBits::eCubeCompatible;

            shadowCubeMap.assignRes(engine->_device().createImage(shadowCubeMapInfo), engine->_device());

            vk::ImageViewCreateInfo shadowCubeMapViewInfo{};
            shadowCubeMapViewInfo.image = shadowCubeMap;
            shadowCubeMapViewInfo.viewType = vk::ImageViewType::eCube;
            shadowCubeMapViewInfo.format = vk::Format::eD32Sfloat;
            vk::ImageSubresourceRange shadowCubeMapSubresourceRange{};
            shadowCubeMapSubresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
            shadowCubeMapSubresourceRange.baseMipLevel = 0;
            shadowCubeMapSubresourceRange.levelCount = 1;
            shadowCubeMapSubresourceRange.baseArrayLayer = 0;
            shadowCubeMapSubresourceRange.layerCount = 6;
            shadowCubeMapViewInfo.subresourceRange = shadowCubeMapSubresourceRange;

            shadowCubeMapView.assignRes(engine->_device().createImageView(shadowCubeMapViewInfo), engine->_device());

            for (uint32_t face = 0; face < 6; ++face)
            {
                vk::ImageViewCreateInfo faceViewInfo{};

                faceViewInfo
                    .setImage(shadowCubeMap)
                    .setViewType(vk::ImageViewType::e2D)
                    .setFormat(vk::Format::eD32Sfloat)
                    .setSubresourceRange(vk::ImageSubresourceRange{
                        vk::ImageAspectFlagBits::eDepth,
                        0,      // baseMipLevel
                        1,      // levelCount
                        face,   // baseArrayLayer
                        1       // layerCount
                    });

                shadowViews[face].assignRes(engine->_device().createImageView(faceViewInfo), engine->_device());
            }

            constexpr glm::vec3 lightPos = glm::vec3(-10.0f, 0.0f, -60.0f);

            shadowViewMatricies[0] = glm::lookAt(
                lightPos,
                lightPos + glm::vec3{ 1, 0, 0 },
                glm::vec3{ 0,-1, 0 });

            shadowViewMatricies[1] = glm::lookAt(
                lightPos,
                lightPos + glm::vec3{-1, 0, 0},
                glm::vec3{ 0,-1, 0 });

            shadowViewMatricies[2] = glm::lookAt(
                lightPos,
                lightPos + glm::vec3{ 0, 1, 0 },
                glm::vec3{ 0, 0, 1 });

            shadowViewMatricies[3] = glm::lookAt(
                lightPos,
                lightPos + glm::vec3{ 0,-1, 0 },
                glm::vec3{ 0, 0,-1 });

            shadowViewMatricies[4] = glm::lookAt(
                lightPos,
                lightPos + glm::vec3{ 0, 0, 1 },
                glm::vec3{ 0,-1, 0 });

            shadowViewMatricies[5] = glm::lookAt(
                lightPos,
                lightPos + glm::vec3{ 0, 0,-1 },
                glm::vec3{ 0,-1, 0 });

            
        }

        ~Scene()
        {
            // for(const auto& b : instanceBuffers)
            // {
            //     b.second->~Buffer();
            // }
        }
    };
}

#include "Scene.tpp"