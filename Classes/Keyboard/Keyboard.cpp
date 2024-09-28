#include "Keyboard.hpp"
#include "nihil-render/Classes/App/App.hpp"

void nihil::Keyboard::registerKey(Key key)
{
	registeredKeys.insert(key);
	keys.insert(std::make_pair(key, false));
}

bool nihil::Keyboard::checkKey(Key key)
{
	if (registeredKeys.find(key) == registeredKeys.end()) { throw std::exception("Can't check unregistered key."); return false; };

	//check if key is pressed
	return keys.find(key)->second;
}

void nihil::Keyboard::registerKeyCallback(Key key, nstd::Callable<nstd::CallableTemplate<KeyCallback, KeyCallbackReturn, Key>> callback)
{

}

nihil::Keyboard::Keyboard(App* app)
{
	glfwMakeContextCurrent(app->get->window);

	glfwSetWindowUserPointer(app->get->window, this);

	glfwSetKeyCallback(app->get->window, keyCallback);
}

void nihil::Keyboard::setKeyState(Key key, bool state)
{
	keys[key] = state;
}