#pragma once

#include <iostream>
#include <unordered_set>
#include <GLFW/glfw3.h>

constexpr size_t size_char = sizeof(char);

#include "nstd/nstd.hpp"

constexpr unsigned long long maxNumberForBytes(unsigned int numBytes) {
	unsigned int numBits = numBytes * 8;
	if (numBits == 0) return 0;
	return (1ULL << numBits) - 1;
}

namespace nihil
{
	class App;
	/*
	* Characters apart from the function keys and chord keys, Tab, Space, Enter, Backspace ETC. 
	* can be accesed by: (Key)'/your key/'; so by casting a char to the Key type
	* 
	* arrows will be added
	*/
	enum class Key
	{
		q = (int)'q', w = (int)'w', e = (int)'e', r = (int)'r', t = (int)'t', y = (int)'y', u = (int)'u', i = (int)'i', o = (int)'o', p = (int)'p',
		a = (int)'a', s = (int)'s', d = (int)'d', f = (int)'f', g = (int)'g', h = (int)'h', j = (int)'j', k = (int)'k', l = (int)'l',
		z = (int)'z', x = (int)'x', c = (int)'c', v = (int)'v', b = (int)'b', n = (int)'n', m = (int)'m',

		Esc = maxNumberForBytes(size_char) + 1, Tab = maxNumberForBytes(size_char) + 2,
		Backspace = maxNumberForBytes(size_char) + 3, Enter = maxNumberForBytes(size_char) + 4,
		LShif = maxNumberForBytes(size_char) + 5, RShift = maxNumberForBytes(size_char) + 6,
		Shift = maxNumberForBytes(size_char) + 7, LCtrl = maxNumberForBytes(size_char) + 8,
		RCtrl = maxNumberForBytes(size_char) + 9, Ctrl = maxNumberForBytes(size_char) + 10,
		LAlt = maxNumberForBytes(size_char) + 11, RAlt = maxNumberForBytes(size_char) + 12, Alt = maxNumberForBytes(size_char) + 13,

		F1 = maxNumberForBytes(size_char) + 14, F2 = maxNumberForBytes(size_char) + 15,
		F3 = maxNumberForBytes(size_char) + 16, F4 = maxNumberForBytes(size_char) + 17,
		F5 = maxNumberForBytes(size_char) + 18, F6 = maxNumberForBytes(size_char) + 19,
		F7 = maxNumberForBytes(size_char) + 20, F8 = maxNumberForBytes(size_char) + 21,
		F9 = maxNumberForBytes(size_char) + 22, F10 = maxNumberForBytes(size_char) + 23,
		F11 = maxNumberForBytes(size_char) + 24, F12 = maxNumberForBytes(size_char) + 25,

		Tilde = (int)'~', Backtick = (int)'`', Colon = (int)':', SemiColon = (int)';', Apostrophe = (int)'\'', DoubleQuote = (int)'\"',
		Minus = (int)'-', Plus = (int)'+', ChevronOpen = (int)'<', ChevronClose = (int)'>', Coma = (int)',', Dot = (int)'.',
		ForwardSlash = (int)'/', BackSlash = (int)'\\', QuestionMark = (int)'?', Or = (int)'|', Exclamation = (int)'!',
		At = (int)'@', Hash = (int)'#', Dollar = (int)'$', Percent = (int)'%', Circumflex = (int)'^', And = (int)'&', Asterisk = (int)'*',

		CurlyBracketOpen = (int)'{', CurlyBracketClose = (int)'}',
		SquareBracketOpen = (int)'[', SquareBracketClose = (int)']',
		ParenthesesOpen = (int)'(', ParenthesesClose = (int)')',

		ArrowUp = maxNumberForBytes(size_char) + 26, ArrowDown = maxNumberForBytes(size_char) + 27,
		ArrowLeft = maxNumberForBytes(size_char) + 28, ArrowRight = maxNumberForBytes(size_char) + 29
	};

	struct KeyCallbackReturn
	{
		unsigned char type;
		void* data;
	};
	typedef KeyCallbackReturn(*KeyCallback)(Key);

	class Keyboard
	{
	private:
		std::unordered_set<Key> registeredKeys;
		std::unordered_map<Key, bool> keys;
	public:
		void registerKey(Key key);
		bool checkKey(Key key);
		void setKeyState(Key key, bool state);
		//cursed, since its using the nstd::Callable. still usable
		void registerKeyCallback(Key key, nstd::Callable<nstd::CallableTemplate<KeyCallback, KeyCallbackReturn, Key>> callback);

		Keyboard(App* app);
	};

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		Keyboard* keyboard = static_cast<Keyboard*>(glfwGetWindowUserPointer(window));
		if (action == GLFW_PRESS) {
			switch (key) {
			case GLFW_KEY_Q:
				keyboard->setKeyState(Key::q, true);
				break;
			case GLFW_KEY_LEFT:
				keyboard->setKeyState(Key::ArrowLeft, true);
				break;
			case GLFW_KEY_RIGHT:
				keyboard->setKeyState(Key::ArrowRight, true);
				break;
			case GLFW_KEY_DOWN:
				keyboard->setKeyState(Key::ArrowDown, true);
				break;
			case GLFW_KEY_UP:
				keyboard->setKeyState(Key::ArrowUp, true);
				break;
			case GLFW_KEY_W:
				keyboard->setKeyState(Key::w, true);
				break;
			case GLFW_KEY_S:
				keyboard->setKeyState(Key::s, true);
				break;
			case GLFW_KEY_A:
				keyboard->setKeyState(Key::a, true);
				break;
			case GLFW_KEY_D:
				keyboard->setKeyState(Key::d, true);
				break;
			default:

				break;
			}
		}
		else if (action == GLFW_RELEASE) {
			switch (key) {
			case GLFW_KEY_Q:
				keyboard->setKeyState(Key::q, false);
				break;
			case GLFW_KEY_LEFT:
				keyboard->setKeyState(Key::ArrowLeft, false);
				break;
			case GLFW_KEY_RIGHT:
				keyboard->setKeyState(Key::ArrowRight, false);
				break;
			case GLFW_KEY_DOWN:
				keyboard->setKeyState(Key::ArrowDown, false);
				break;
			case GLFW_KEY_UP:
				keyboard->setKeyState(Key::ArrowUp, false);
				break;
			case GLFW_KEY_W:
				keyboard->setKeyState(Key::w, false);
				break;
			case GLFW_KEY_S:
				keyboard->setKeyState(Key::s, false);
				break;
			case GLFW_KEY_A:
				keyboard->setKeyState(Key::a, false);
				break;
			case GLFW_KEY_D:
				keyboard->setKeyState(Key::d, false);
				break;
			default:

				break;
			}
		}
		else if (action == GLFW_REPEAT) {
			std::cout << "Key repeated: " << key << std::endl;
		}
	}
}