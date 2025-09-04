#pragma once

#include "Classes/Model/Model.hpp"
#include "Classes/Buffer/Buffer.hpp"
#include "Classes/Engine/Engine.hpp"
#include "Classes/Material/Material.hpp"

#include "Classes/Camera/Camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Classes/Asset/Asset.hpp"

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

        PushConstants pushConstants;

        //TODO: allow multiple models in one object
        Model* model = nullptr;
        Material* material = nullptr;
        Engine* engine = nullptr;

        uint64_t modelMaterialEncoded = 0;

        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 rotation = glm::vec3(0.0f);
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        inline Model* _model() const { return model; };
        inline const glm::vec3& _position() { return position; };
        inline const glm::vec3& _rotation() { return rotation; };
        inline const glm::mat4& _modelMatrix() { return modelMatrix; };

        inline uint64_t _modelMaterialEncoded() { return modelMaterialEncoded; };

        inline const PushConstants* _pushConstants(Camera* camera) 
        {
            pushConstants.model = modelMatrix;
            pushConstants.vp = camera->_vp();

            return &pushConstants;
        }

        inline Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>& _vertexBuffer() { return model->_vertexBuffer(); };
        inline Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer>& _indexBuffer() { return model->_indexBuffer(); };

        inline Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* _vertexBufferPtr() { return model->_vertexBufferPtr(); };
        inline Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer>* _indexBufferPtr() { return model->_indexBufferPtr(); };

        inline void setPosition(const glm::vec3& _position) { position = _position; recalculateModelMatrix(); }; 
        inline void setRotation(const glm::vec3& _rotation) { rotation = _rotation; recalculateModelMatrix(); }; 

        Object(Model* _model, Material* _material, Engine* _engine) : Asset(AssetUsage::Undefined, _engine)
        {
            assert(_model != nullptr);
            assert(_material != nullptr);
            assert(_engine != nullptr);

            model = _model;
            material = _material;
            engine = _engine;

            recalculateModelMatrix();

            modelMaterialEncoded = (static_cast<uint64_t>(model->_getAssetId()) << 32) + material->_getAssetId();
        }

        inline void recalculateModelMatrix()
        {
            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, position);
            modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

            modelMatrix *= model->_deafultTransform();
        }

        //TODO: Physics integration later
        inline void move(const glm::vec3& _moveBy)
        {
            position += _moveBy;
            recalculateModelMatrix();

            modifiedThisFrame = true;
        }

        inline void rotate(const glm::vec3& _rotateBy)
        {
            rotation += _rotateBy;
            recalculateModelMatrix();

            modifiedThisFrame = true;
        }

        inline void modified()
        {
            modifiedThisFrame = true;
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