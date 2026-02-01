#include "App.hpp"
#include "GLFW/glfw3.h"

using namespace nihil;

App::App(std::string _name, uint16_t _width, uint16_t _height)
{
    Carbo::Logger::Init();

    name = _name;
    width = _width;
    height = _height;

    if (!glfwInit()) {
        Carbo::Logger::Exception("Failed to initialize GLFW");
    }

    if (!glfwVulkanSupported()) {
        Carbo::Logger::Exception("Vulkan is not supported on this system");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Disable OpenGL context
    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    if (!window) {
        Carbo::Logger::Exception("Failed to create GLFW window");
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
    fixedOnHandle();
}

inline void App::fixedOnResize()
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

inline void App::fixedOnHandle()
{
    for (auto l : onHandleListeners)
    {
        l->onHandle();
    }
}