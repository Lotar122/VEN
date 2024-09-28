#pragma once
#include <GLFW/glfw3.h>
#include "Classes/Scene/Scene.hpp"
#include "Classes/Engine/Engine.Structs.InData.hpp"
#include "nstd/nstd.hpp"

#include <iostream>

namespace nihil::graphics {
	class Engine;
}

namespace nihil {
	struct AppCreationArgs {
		uint16_t width;
		uint16_t height;
		std::string name;
		graphics::Version vulkanVersion;
		graphics::Version appVersion;
		graphics::Engine* engine;
	};

	class App
	{
	private:
		struct Proxy {
			const uint16_t* width;
			const uint16_t* height;
			const std::string* name;
			graphics::Engine* engine;
			graphics::Scene* scene;
			GLFWwindow* window;
			const bool* shouldClose;
			const graphics::Version* vulkanVersion;
			const graphics::Version* appVersion;
			Proxy(
				const uint16_t* width,
				const uint16_t* height,
				const std::string* name,
				graphics::Engine* engine,
				graphics::Scene* scene,
				GLFWwindow* window,
				const bool* shouldClose,
				const graphics::Version* appVersion,
				const graphics::Version* vulkanVersion
			)
			{
				this->width = width;
				this->height = height;
				this->name = name;
				this->engine = engine;
				this->scene = scene;
				this->window = window;
				this->shouldClose = shouldClose;
				this->appVersion = appVersion;
				this->vulkanVersion = vulkanVersion;
			}
		};
		uint16_t width;
		uint16_t height;
		std::string name;

		graphics::Engine* engine;
		graphics::Scene* scene;

		GLFWwindow* window;

		graphics::Version vulkanVersion;
		graphics::Version appVersion;

		bool shouldClose = false;

		int gcd(int a, int b) {
			while (b != 0) {
				int temp = b;
				b = a % b;
				a = temp;
			}
			return a;
		}
	public:
		nstd::ScreenRatio screenRatio;
		Proxy* get;
		App(AppCreationArgs* args);
		~App();

		void handle();

		inline void SetWidth(uint16_t width) {
			this->width = width;
		}
		inline void SetHeight(uint16_t height) {
			this->height = height;
		}
	};
}

#include "Classes/Engine/Engine.hpp"