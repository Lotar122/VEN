#pragma once

#include "Classes/Model/Model.hpp"
#include "Classes/Buffer/Buffer.hpp"
#include "Classes/Engine/Engine.hpp"
#include "Classes/Material/Material.hpp"

#include "Classes/Camera/Camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Classes/Asset/Asset.hpp"

#include "Classes/AABB/AABB.hpp"
#include "Classes/ObjectAllocator/ObjectAllocator.hpp"

namespace nihil::graphics
{
    class Scene;

    class Object : public Asset
    {
        friend class Scene;
    public:
        //? used for resource optimization in the future, for now just sits here to remind me of my plans.
        bool active = true;

        bool modifiedThisFrame = false;

        size_t prevDataSlot = std::numeric_limits<size_t>::max();
        uint64_t lastModifiedFrame = std::numeric_limits<uint64_t>::max();

        PushConstants pushConstants;

        //TODO: allow multiple models in one object
        Model* model = nullptr;
        Material* material = nullptr;
        ObjectAllocator* objectAllocator = nullptr;
        Engine* engine = nullptr;

        uint64_t modelMaterialEncoded = 0;

        glm::vec3 scaleFactor = glm::vec3(1.0f);
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 rotation = glm::vec3(0.0f);
        size_t modelMatrix = 0;
        //glm::mat4 modelMatrix = glm::mat4(1.0f);

        size_t aabb;
        size_t transformedAABB;

        bool autoCreatedInstanceData = false;
        const void* instanceData = nullptr;
        size_t instanceDataSize = 0;

        inline Model* _model() const { return model; };
        inline const glm::vec3& _position() { return position; };
        inline const glm::vec3& _rotation() { return rotation; };
        inline const glm::mat4& _modelMatrix() { return objectAllocator->modelMatricies.at(modelMatrix); };
        inline Material* _material() { return material; };

        inline uint64_t _modelMaterialEncoded() { return modelMaterialEncoded; };

        inline AABB& _transformedAABB() { return objectAllocator->transformedAABBs.at(transformedAABB); };
        inline AABB& _aabb() { return objectAllocator->AABBs.at(aabb); }

        inline const PushConstants* _pushConstants(Camera* camera) 
        {
            pushConstants.model = modelMatrix;
            pushConstants.vp = camera->_vp();

            return &pushConstants;
        }

        inline Buffer<std::vector<float>, vk::BufferUsageFlagBits::eVertexBuffer>& _vertexBuffer() { return model->_vertexBuffer(); };
        inline Buffer<std::vector<uint32_t>, vk::BufferUsageFlagBits::eIndexBuffer>& _indexBuffer() { return model->_indexBuffer(); };

        inline Buffer<std::vector<float>, vk::BufferUsageFlagBits::eVertexBuffer>* _vertexBufferPtr() { return model->_vertexBufferPtr(); };
        inline Buffer<std::vector<uint32_t>, vk::BufferUsageFlagBits::eIndexBuffer>* _indexBufferPtr() { return model->_indexBufferPtr(); };

        inline void setPosition(const glm::vec3& _position) { position = _position; recalculateModelMatrix(); }; 
        inline void setRotation(const glm::vec3& _rotation) { rotation = _rotation; recalculateModelMatrix(); }; 

        Object(Model* _model, Material* _material, ObjectAllocator* _allocator, Engine* _engine) : Asset(AssetUsage::Undefined, _engine)
        {
            assert(_model != nullptr);
            assert(_material != nullptr);
            assert(_allocator != nullptr);
            assert(_engine != nullptr);

            model = _model;
            material = _material;
            objectAllocator = _allocator;
            engine = _engine;

            aabb = objectAllocator->AABBs.allocate(model->_aabb());
            transformedAABB = objectAllocator->transformedAABBs.allocate();
            modelMatrix = objectAllocator->modelMatricies.allocate(1.0f);

            recalculateModelMatrix();

            modelMaterialEncoded = (static_cast<uint64_t>(model->_getAssetId()) << 32) + material->_getAssetId();
        }

        template<bool warn = true>
        inline void setInstanceData(void* _instanceData, size_t _instanceDataSize)
        {
            if constexpr (warn)
            {
                if (instanceData != nullptr && !autoCreatedInstanceData) Carbo::Logger::Warn("Chainging the instance data of object: {:p} ensure that you have deleted the previous instance data: {:p}. Not doing so may cause a memory leak.", static_cast<const void*>(this), instanceData);
            }

            autoCreatedInstanceData = false;
            instanceData = _instanceData;
            instanceDataSize = _instanceDataSize;
        }

        std::pair<const void*, size_t> _instanceData()
        {
            const bool hasData = instanceData != nullptr;
            const bool hasSize = instanceDataSize > 0;

            if (hasData != hasSize) [[unlikely]]
            {
                Carbo::Logger::Exception("The instance data of object: {:p} is invalid", static_cast<const void*>(this));
            }

            if (hasData)
            {
                return std::make_pair(instanceData, instanceDataSize);
            }
            else
            {
                //Since no data was provided by the user create the instanceData

                constexpr size_t instanceDataChunkSize = 16 * sizeof(float);

                autoCreatedInstanceData = true;
                instanceData = reinterpret_cast<const void*>(glm::value_ptr(objectAllocator->modelMatricies.at(modelMatrix)));
                instanceDataSize = instanceDataChunkSize;

                return std::make_pair(instanceData, instanceDataSize);
            }
        }

        inline void recalculateModelMatrix()
        {
            glm::mat4& modelMatrixRef = objectAllocator->modelMatricies.at(modelMatrix);
            modelMatrixRef = glm::mat4(1.0f);
            modelMatrixRef = glm::translate(modelMatrixRef, position);
            modelMatrixRef = glm::rotate(modelMatrixRef, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            modelMatrixRef = glm::rotate(modelMatrixRef, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            modelMatrixRef = glm::rotate(modelMatrixRef, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            modelMatrixRef = glm::scale(modelMatrixRef, scaleFactor);

            modelMatrixRef *= model->_defaultTransform();

            objectAllocator->transformedAABBs.at(transformedAABB) = objectAllocator->AABBs.at(aabb).getTransformed(modelMatrixRef);
        }

        //TODO: Physics integration later
        inline void move(const glm::vec3& _moveBy)
        {
            position += _moveBy;
            recalculateModelMatrix();

            lastModifiedFrame = engine->_currentFrame();
        }

        inline void rotate(const glm::vec3& _rotateBy)
        {
            rotation += _rotateBy;
            recalculateModelMatrix();

            lastModifiedFrame = engine->_currentFrame();
        }

        inline void scale(const float _scale)
        {
            scaleFactor *= _scale;
            recalculateModelMatrix();

            lastModifiedFrame = engine->_currentFrame();
        }

        inline void scale(const glm::vec3& _scale)
        {
            scaleFactor *= _scale;
            recalculateModelMatrix();

            lastModifiedFrame = engine->_currentFrame();
        }

        inline void modified()
        {
            lastModifiedFrame = engine->_currentFrame();
        }

        inline void use() { model->moveToGPU(); };
        inline void unuse() { model->freeFromGPU(); };

    private:
        inline void afterRender()
        {
            modifiedThisFrame = false;
        }
    };
}