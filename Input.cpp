#include "Input.h"
#include <SDL.h>

std::unordered_map<std::string, Controls::InputState> Controls::inputValues;
std::unordered_map<int, std::string> Controls::inputNameMap;

Controls::InputState Controls::getInput(std::string s)
{
	if (inputValues.count(s) > 0) return inputValues[s];
	return Controls::InputState::Free;
}

bool Controls::getInputDown(std::string s)
{
	if (inputValues.count(s) <= 0) return false;
	InputState is = inputValues[s];
	if (is == InputState::Held || is == InputState::Pressed) return true;
	return false;
}

void Controls::addInput(std::string inputName, SDL_Keycode keycode)
{
	inputNameMap.insert(std::pair<int, std::string>((int)keycode, inputName));
	inputValues.insert(std::pair<std::string, InputState>(inputName, InputState::Free));
}

void Controls::changeInput(std::string inputName, SDL_Keycode newKeycode)
{
	auto it = inputNameMap.begin();
	//find inputname value
	while (it != inputNameMap.end())
	{
		if (it->second == inputName) break;
		it++;
	}
	if (it == inputNameMap.end())
		return;
	inputNameMap.erase(it);
	inputNameMap.insert(std::pair<int, std::string>((int)newKeycode, inputName));
}

bool Controls::proccessKeyEvent(SDL_Event& e)
{
	if (e.type == SDL_KEYDOWN)
	{
		SDL_Keycode keycode = e.key.keysym.sym;
		if (inputNameMap.count((int)keycode) > 0)
		{
			std::string inputName = inputNameMap[(int)keycode];
			InputState& value = inputValues[inputName];
			if (value == InputState::Free) value = InputState::Pressed;
			else if (value == InputState::Released) value = InputState::Pressed;
			else value = InputState::Held;
			return true;
		}
	}


	if (e.type == SDL_KEYUP)
	{
		SDL_Keycode keycode = e.key.keysym.sym;
		if (inputNameMap.count((int)keycode) > 0)
		{
			std::string inputName = inputNameMap[(int)keycode];
			InputState& value = inputValues[inputName];
			if (value == InputState::Held) value = InputState::Released;
			else if (value == InputState::Pressed) value = InputState::Released;
			else value = InputState::Free;
			return true;
		}
	}

	return false;
}

void Controls::resetFrame()
{
	for (auto& p : inputValues)
	{
		if (p.second == InputState::Pressed) p.second = InputState::Held;
		if (p.second == InputState::Released) p.second = InputState::Free;
	}
}
