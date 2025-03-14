#include "Scene.hpp"

#include "Classes/Pipeline/Pipeline.hpp"

using namespace nihil::graphics;

void Scene::recordCommands(vk::CommandBuffer& commandBuffer, Camera* camera)
{
    /*
    TODO:
    Group the objects by their model, then check if the model has an instanced pipeline and render the model using the instanced pipeline.
    */

    //* For now just loop over them and render them

    for(Object* o : objects)
    {
        //? There will be per model push constants in the future
        //TODO: Add per model render passes

        //Push Constants
        commandBuffer.pushConstants(o->model->_pipeline()->_layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants), o->_pushConstants(camera));

        //Draw
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, o->model->_pipeline()->_pipeline());
        commandBuffer.bindVertexBuffers(0, o->_vertexBuffer()._buffer(), {0});

        commandBuffer.bindIndexBuffer(o->_indexBuffer()._buffer(), 0, vk::IndexType::eUint32);

        commandBuffer.drawIndexed(static_cast<uint32_t>(o->_indexBuffer()._typedSize()), 1, 0, 0, 0);
    }
}