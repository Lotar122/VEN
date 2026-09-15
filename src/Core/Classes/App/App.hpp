#pragma once

#define GLFW_INCLUDE_VULKAN

#include <cstdint>
#include <GLFW/glfw3.h>
#include <functional>
#include <cassert>
#include <mutex>

#include "Classes/Logger/Logger.hpp"

#include "Classes/Listeners/Listeners.hpp"

namespace nihil
{
    class App;

    struct WindowPointer
    {
        App* app = nullptr;
    };

    ///The class that represents the application
    class App
    {
    public:
        ///The constructor for App
        //
        ///@param _name The application name, this is also the window title
        ///@param _width The initial width of the window
        ///@param _height The initial height of the window
        App(std::string _name, uint16_t _width, uint16_t _height);
        ///The default constructor
        ~App() {};

        ///The apps std::mutex
        std::mutex mtx;

        ///The application name
        std::string name = "nihil based application";
        
        //implement the versioning
        //vkVersion
        //appVersion

        ///The windows width
        uint16_t width;
        ///The windows height
        uint16_t height;

        //TODO: Make this atomic
        ///The event loop flag
        bool shouldExit = false;

        ///The GLFWwindow pointer
        GLFWwindow* window = nullptr;
        ///The pointer for context to GLFW events
        WindowPointer wp = { this };

        //The onResize event listener
        //std::function<void(App*, void*)> onResize = [](App* app, void* usrPtr) {};
        //The user provided context for events
        //void* userPointer = nullptr;

        ///The default handle function
        void handle();

        ///Acquire the app's std::mutex mtx
        inline void access() { mtx.lock(); };
        ///Unlock the app's std::mutex mtx
        inline void endAccess() { mtx.unlock(); };

        ///Adds an event listener
        //
        ///@tparam T The type of the listener
        ///@param type The type of the listener
        template<typename T>
        void addEventListener(T* listener, Listeners type);

    private:
        ///The fixed onResize handler
        inline void fixedOnResize();
        ///The fixed onHandle handler
        inline void fixedOnHandle();

        ///std::vector storing the listeners for onResize
        std::vector<onResizeListener*> onResizeListeners;
        ///std::vector storing the listeners for onHandle
        std::vector<onHandleListener*> onHandleListeners;
    };
}

#include "App.tpp"