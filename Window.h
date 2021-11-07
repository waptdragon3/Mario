#pragma once
//#include <SDL.h>
#include "Math.h"
#include <string>
struct SDL_Window;
struct SDL_Surface;

class Window
{
private:
	SDL_Window* iWindow;
	SDL_Surface* screenSurface;

	std::string title;

public:
	Window(std::string _title, Vec2 size);

	bool isValid();
	Vec2 getSize();
	void finishRendering();

	void quit();

};