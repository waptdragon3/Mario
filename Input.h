#pragma once
#include "SDL_keycode.h"
#include <unordered_map>

union SDL_Event;

class Controls
{
public:
	/// <summary>
	/// Held: The key is held down
	/// Pressed: First frame of a key being held
	/// Released: Last frame of a key being held
	/// </summary>
	enum class InputState { Free = 1, Held = 2, Pressed = 4, Released = 8 };
private:
	static std::unordered_map<std::string, InputState> inputValues;
	static std::unordered_map<int, std::string> inputNameMap;
public:
	static InputState getInput(std::string);
	static bool getInputDown(std::string);
	static void addInput(std::string inputName, SDL_Keycode keycode);
	static void changeInput(std::string inputName, SDL_Keycode newKeycode);

	//returns true if the event was used
	static bool proccessKeyEvent(SDL_Event& e);
	static void resetFrame();
};