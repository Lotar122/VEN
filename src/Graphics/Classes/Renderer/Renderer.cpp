#include "Renderer.hpp"

#include "Classes/Engine/Engine.hpp"
#include "Classes/Swapchain/Swapchain.hpp"

using namespace nihil::graphics;

Renderer::Renderer(Engine* _engine)
{
    assert(_engine != nullptr);

    engine = _engine;

    engine->_app()->access();

    engine->_app()->addEventListener(this, Listeners::onResize);

    engine->_app()->endAccess();

    //for now only initialize the viewport and scissor in here

    viewport = vk::Viewport();
    viewport.x = 0.0f;                // Top-left corner X
    viewport.y = 0.0f;                // Top-left corner Y
    viewport.width = static_cast<float>(engine->swapchain->_width());  // Viewport width
    viewport.height = static_cast<float>(engine->swapchain->_height()); // Viewport height
    viewport.minDepth = 0.0f;          // Minimum depth value
    viewport.maxDepth = 1.0f;          // Maximum depth value

    scissor = vk::Rect2D();
    scissor.offset = vk::Offset2D{0, 0};  // Top-left corner
    scissor.extent = vk::Extent2D{engine->swapchain->_width(), engine->swapchain->_height()}; // Size of the scissor
}

void Renderer::onResize()
{
    engine->_app()->access();

	viewport = vk::Viewport();
    viewport.x = 0.0f;                // Top-left corner X
    viewport.y = 0.0f;                // Top-left corner Y
    viewport.width = static_cast<float>(engine->_app()->width);  // Viewport width
    viewport.height = static_cast<float>(engine->_app()->height); // Viewport height
    viewport.minDepth = 0.0f;          // Minimum depth value
    viewport.maxDepth = 1.0f;          // Maximum depth value

    scissor = vk::Rect2D();
    scissor.offset = vk::Offset2D{0, 0};  // Top-left corner
    scissor.extent = vk::Extent2D{engine->_app()->width, engine->_app()->height}; // Size of the scissor

    engine->_app()->endAccess();
}