#include "Window.h"
#include <SDL.h>
#include <iostream>

Window::Window(std::string _title, Vec2 size)
{
	title = _title;
	iWindow = SDL_CreateWindow(_title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (int)size.X, (int)size.Y, SDL_WINDOW_SHOWN);
	if (iWindow == NULL)
	{
		std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(iWindow);
	}
	else
	{
		screenSurface = SDL_GetWindowSurface(iWindow);
	}
}

bool Window::isValid()
{
	return iWindow != NULL;
}

Vec2 Window::getSize()
{
	int x, y;
	SDL_GetWindowSize(iWindow, &x, &y);
	return Vec2((float)x, (float)y);
}

void Window::finishRendering()
{
	SDL_UpdateWindowSurface(iWindow);
}

void Window::quit()
{
	SDL_DestroyWindow(iWindow);
}
