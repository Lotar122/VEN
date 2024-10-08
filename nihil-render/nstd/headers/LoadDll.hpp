#pragma once

#include <string>
#include <stdexcept>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

class DynamicLibrary {
public:
    explicit DynamicLibrary(const std::string& libraryName) {
        loadLibrary(libraryName);
    }

    ~DynamicLibrary() {
        unloadLibrary();
    }

    void* getFunction(const std::string& functionName) const {
#ifdef _WIN32
        return GetProcAddress(static_cast<HMODULE>(handle), functionName.c_str());
#else
        return dlsym(handle, functionName.c_str());
#endif
    }

    bool isLoaded() const {
        return handle != nullptr;
    }

private:
    void* handle{ nullptr };

    void loadLibrary(const std::string& libraryName) {
#ifdef _WIN32
        handle = LoadLibraryA(libraryName.c_str());
#else
        handle = dlopen(libraryName.c_str(), RTLD_LAZY);
        if (!handle) {
            std::cerr << "Error loading library: " << dlerror() << std::endl;
        }
#endif
        if (!handle) {
            throw std::runtime_error("Failed to load library: " + libraryName);
        }
    }

    void unloadLibrary() {
        if (handle) {
#ifdef _WIN32
            FreeLibrary(static_cast<HMODULE>(handle));
#else
            dlclose(handle);
#endif
            handle = nullptr;
        }
    }
};