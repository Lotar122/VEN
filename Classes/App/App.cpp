#include "App.hpp"

using namespace nihil;

App::App(std::string _name, uint16_t _width, uint16_t _height)
{
    name = _name;
    width = _width;
    height = _height;

    if (!glfwInit()) {
        Logger::Exception("Failed to initialize GLFW");
    }

    if (!glfwVulkanSupported()) {
        Logger::Exception("Vulkan is not supported on this system");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Disable OpenGL context
    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    if (!window) {
        Logger::Exception("Failed to create GLFW window");
    }

    glfwSetWindowUserPointer(window, &wp);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        WindowPointer* windowPointer = reinterpret_cast<WindowPointer*>(glfwGetWindowUserPointer(window));
        windowPointer->app->fixedOnResize();
    });
}

void App::handle()
{
    glfwPollEvents();
    shouldExit = glfwWindowShouldClose(window);
}

void App::fixedOnResize()
{
    // Get the width and height of the window
    int _width, _height;
    glfwGetWindowSize(window, &_width, &_height);

    width = _width;
    height = _height;

    onResize(this, userPointer);

    for(auto l : onResizeListeners)
    {
        l->onResize();
    }
}