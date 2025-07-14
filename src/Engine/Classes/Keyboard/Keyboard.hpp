#pragma once

#include <cstdint>
#include <unordered_map>
#include <cassert>

#include "Classes/App/App.hpp"
#include "Classes/Listeners/Listeners.hpp"

namespace nihil
{
	enum class Key {
		A = GLFW_KEY_A, B = GLFW_KEY_B, C = GLFW_KEY_C, D = GLFW_KEY_D, E = GLFW_KEY_E,
		F = GLFW_KEY_F, G = GLFW_KEY_G, H = GLFW_KEY_H, I = GLFW_KEY_I, J = GLFW_KEY_J,
		K = GLFW_KEY_K, L = GLFW_KEY_L, M = GLFW_KEY_M, N = GLFW_KEY_N, O = GLFW_KEY_O,
		P = GLFW_KEY_P, Q = GLFW_KEY_Q, R = GLFW_KEY_R, S = GLFW_KEY_S, T = GLFW_KEY_T,
		U = GLFW_KEY_U, V = GLFW_KEY_V, W = GLFW_KEY_W, X = GLFW_KEY_X, Y = GLFW_KEY_Y, Z = GLFW_KEY_Z,
		Num0 = GLFW_KEY_0, Num1 = GLFW_KEY_1, Num2 = GLFW_KEY_2, Num3 = GLFW_KEY_3, Num4 = GLFW_KEY_4,
		Num5 = GLFW_KEY_5, Num6 = GLFW_KEY_6, Num7 = GLFW_KEY_7, Num8 = GLFW_KEY_8, Num9 = GLFW_KEY_9,
		Space = GLFW_KEY_SPACE, Escape = GLFW_KEY_ESCAPE, Enter = GLFW_KEY_ENTER, Tab = GLFW_KEY_TAB,
		Backspace = GLFW_KEY_BACKSPACE, Insert = GLFW_KEY_INSERT, Delete = GLFW_KEY_DELETE,
		ArrowRight = GLFW_KEY_RIGHT, ArrowLeft = GLFW_KEY_LEFT, ArrowDown = GLFW_KEY_DOWN, ArrowUp = GLFW_KEY_UP,
		PageUp = GLFW_KEY_PAGE_UP, PageDown = GLFW_KEY_PAGE_DOWN, Home = GLFW_KEY_HOME, End = GLFW_KEY_END,
		CapsLock = GLFW_KEY_CAPS_LOCK, ScrollLock = GLFW_KEY_SCROLL_LOCK, NumLock = GLFW_KEY_NUM_LOCK,
		PrintScreen = GLFW_KEY_PRINT_SCREEN, Pause = GLFW_KEY_PAUSE, F1 = GLFW_KEY_F1, F2 = GLFW_KEY_F2,
		F3 = GLFW_KEY_F3, F4 = GLFW_KEY_F4, F5 = GLFW_KEY_F5, F6 = GLFW_KEY_F6, F7 = GLFW_KEY_F7, F8 = GLFW_KEY_F8,
		F9 = GLFW_KEY_F9, F10 = GLFW_KEY_F10, F11 = GLFW_KEY_F11, F12 = GLFW_KEY_F12,
		NumPad0 = GLFW_KEY_KP_0, NumPad1 = GLFW_KEY_KP_1, NumPad2 = GLFW_KEY_KP_2, NumPad3 = GLFW_KEY_KP_3,
		NumPad4 = GLFW_KEY_KP_4, NumPad5 = GLFW_KEY_KP_5, NumPad6 = GLFW_KEY_KP_6, NumPad7 = GLFW_KEY_KP_7,
		NumPad8 = GLFW_KEY_KP_8, NumPad9 = GLFW_KEY_KP_9, NumPadDecimal = GLFW_KEY_KP_DECIMAL,
		NumPadDivide = GLFW_KEY_KP_DIVIDE, NumPadMultiply = GLFW_KEY_KP_MULTIPLY, NumPadSubstract = GLFW_KEY_KP_SUBTRACT,
		NumPadAdd = GLFW_KEY_KP_ADD, NumPadEnter = GLFW_KEY_KP_ENTER, NumPadEqual = GLFW_KEY_KP_EQUAL,
		LShift = GLFW_KEY_LEFT_SHIFT, LCtrl = GLFW_KEY_LEFT_CONTROL, LAlt = GLFW_KEY_LEFT_ALT,
		LSuper = GLFW_KEY_LEFT_SUPER, RShift = GLFW_KEY_RIGHT_SHIFT, RCtrl = GLFW_KEY_RIGHT_CONTROL,
		RAlt = GLFW_KEY_RIGHT_ALT, RSuper = GLFW_KEY_RIGHT_SUPER, Menu = GLFW_KEY_MENU,

		Semicolon = GLFW_KEY_SEMICOLON,
		Apostrophe = GLFW_KEY_APOSTROPHE,
		Comma = GLFW_KEY_COMMA,
		Period = GLFW_KEY_PERIOD,
		Slash = GLFW_KEY_SLASH,
		LBracket = GLFW_KEY_LEFT_BRACKET,
		Backslash = GLFW_KEY_BACKSLASH,
		RBracket = GLFW_KEY_RIGHT_BRACKET,

		GraveAccent = GLFW_KEY_GRAVE_ACCENT,

		Minus = GLFW_KEY_MINUS,
		Equal = GLFW_KEY_EQUAL
	};

	class Keyboard : public onHandleListener
	{
		App* app = nullptr;
	public:
		Keyboard(App* _app)
		{
			assert(_app != nullptr);

			app = _app;

			app->addEventListener(this, Listeners::onHandle);
		}

		std::unordered_map<Key, bool> keys;

		inline void useKey(Key key)
		{
			keys.insert(std::make_pair(key, false));
		}

		void onHandle() final override 
		{
			//Handle all the key changes

			for (auto& it : keys)
			{
				it.second = (glfwGetKey(app->window, (uint32_t)it.first) == GLFW_PRESS);
			}
		}

		inline bool getKey(Key key)
		{
			return keys[key];
		}
	};
}