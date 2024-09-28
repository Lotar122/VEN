#include "App.hpp"

using namespace nihil;

App::App(AppCreationArgs* args)
{
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		throw std::exception("Failed to initialize GLFW");
	}
	std::cout << "still here" << std::endl;
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	scene = new graphics::Scene();

	this->engine = args->engine;
	this->width = args->width;
	this->height = args->height;
	this->name = args->name;

	get = new Proxy(
		&width,
		&height,
		&name,
		engine,
		scene,
		window,
		&shouldClose,
		&appVersion,
		&vulkanVersion
	);

	window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);

	// Calculate the greatest common divisor
	//int divisor = gcd(width, height);
	int divisor;
	if (height > width) {
		divisor = height;
	}
	else {
		divisor = width;
	}

	// Calculate simplified aspect ratio
	float aspectRatioWidth = (float)width / (float)divisor;
	float aspectRatioHeight = (float)height / (float)divisor;

	screenRatio.width = aspectRatioWidth;
	screenRatio.height = aspectRatioHeight;

	get->window = window;

	engine->setApp(this);
}
App::~App()
{
	glfwDestroyWindow(window);
	glfwTerminate();

	delete scene;
	delete get;
}
void App::handle()
{
	shouldClose = glfwWindowShouldClose(window);
	glfwPollEvents();
	//engine->Draw(scene);
}