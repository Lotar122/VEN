# Vis Ex Nihil Engine

![Vulkan](https://img.shields.io/badge/Vulkan-API-red.svg)
![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)

A lightweight, high-performance **C++ game engine** built on **Vulkan** for modern graphics rendering. Designed for flexibility, scalability, and efficient rendering.

## Features
✅ **Lightweight & Modular** - Minimal dependencies, easily extensible.
✅ **Vulkan-based Renderer** - Low-level control with high efficiency.

---

## 📦 Installation
### **Prerequisites**
- **CMake** (>= 3.10)
- **C++20 or newer**
- **Vulkan SDK** (Ensure Vulkan drivers are installed)
- **GLFW** (for windowing)
- **GLM** (math library)
- **SPIRV-Cross** (for shader reflection)

### **Build Instructions**
```sh
# Clone the repository
git clone https://github.com/Lotar122/VEN.git
cd VEN

# Create a build directory
mkdir build && cd build

# Generate the project files
cmake .. -DCMAKE_BUILD_TYPE=Release

# Compile the engine
make -j$(nproc)
```

### **Run the Engine**
```sh
./VEN
```

---

## 🚀 Getting Started
### Read The Docs ;)


### **Project Structure**
```
VEN/
├── Resources/      # Resources used by the engine
│   ├── Models/     # Models used by the engine
│   ├── Shaders/    # Shaders used in the engine
│   ├── Textures/   # Textures used by the engine
├── src/            # Source code for the engine
├── thirdparty/     # External dependencies
└── build/          # Compiled binaries
```

---

## 🔧 Configuration
Coming soon.

---

## 🛠️ Dependencies
- [GLFW](https://www.glfw.org/) - Windowing and input handling
- [GLM](https://github.com/g-truc/glm) - Mathematics for graphics
- [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) - Shader reflection

---

## 📝 License
This project is licensed under the **MIT License**.

---

## 🌎 Contributing
Contributions are welcome! Feel free to open an issue or submit a pull request.
```sh
git checkout -b feature-branch
# Make changes
git commit -m "Added new feature"
git push origin feature-branch
```

---

## 📞 Contact
For questions or suggestions, reach out via [GitHub Issues](https://github.com/Lotar122/VEN/issues).

