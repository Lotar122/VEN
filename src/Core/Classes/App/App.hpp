#pragma once

#define GLFW_INCLUDE_VULKAN

#include <cstdint>
#include <GLFW/glfw3.h>
#include <functional>
#include <cassert>
#include <mutex>

#include "Logger.hpp"

#include "Classes/Listeners/Listeners.hpp"

namespace nihil
{
    class App;

    struct WindowPointer
    {
        App* app = nullptr;
    };

    class App
    {
    public:
        App(std::string _name, uint16_t _width, uint16_t _height);
        ~App() {};

        std::mutex mtx;

        std::string name = "nihil based application";
        
        //implement the versioning
        //vkVersion
        //appVersion

        uint16_t width;
        uint16_t height;

        bool shouldExit = false;

        GLFWwindow* window = nullptr;
        WindowPointer wp = { this };

        std::function<void(App*, void*)> onResize = [](App* app, void* usrPtr) {};
        void* userPointer = nullptr;

        void handle();

        inline void access() { mtx.lock(); };
        inline void endAccess() { mtx.unlock(); };

        template<typename T>
        void addEventListener(T* listener, Listeners type);

    private:
        inline void fixedOnResize();
        inline void fixedOnHandle();

        std::vector<onResizeListener*> onResizeListeners;
        std::vector<onHandleListener*> onHandleListeners;
    };
}

#include "App.tpp"