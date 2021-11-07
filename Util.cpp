#include "Util.h"
#include "SDL.h"


class GlobalInfo
{
private:
	Vec2 CameraPosition = Vec2();
	float CameraScale = 5;
public:
	void setCameraPosition(Vec2);
	Vec2 getCameraPosition();

	void setCameraScale(float);
	float getCameraScale();
};

extern bool isPaused;
extern GlobalInfo gGraphicsInfo;

long Timer::prevFrameTime;
long Timer::frameTime;
float Timer::time;

void Timer::resetFrame()
{
	Timer::prevFrameTime = Timer::frameTime;
	Timer::frameTime = SDL_GetTicks();

	Timer::time = (frameTime - prevFrameTime) / 1000.0f;
}

float Timer::getTime()
{
	if (isPaused) return 0;
	//else return .01;
	return time;
}

void Timer::resetTimer()
{
	prevFrameTime = 0;
	frameTime = 0;
	time = 0;
}



extern SDL_Renderer* gRenderer;

std::unordered_map<std::string, Color> Debug::colors;

void Debug::registerColor(std::string s, Color c)
{
	colors.insert(std::pair<std::string, Color>(s, c));
}

Color Debug::getColor(std::string s)
{
	auto f = colors.find(s);
	if (f == colors.end()) return Color();
	else return f->second;
}

void Debug::draw(std::string color, Vec2 p)
{
#ifdef _DEBUG
	Color c = Debug::getColor(color);
	SDL_SetRenderDrawColor(gRenderer, c.R, c.G, c.B, c.A);
	p += gGraphicsInfo.getCameraPosition();

	SDL_RenderDrawPointF(gRenderer, p.X, p.Y);
#endif
}

void Debug::draw(std::string color, Vec2 p1, Vec2 p2)
{
#ifdef _DEBUG
	Color c = Debug::getColor(color);
	SDL_SetRenderDrawColor(gRenderer, c.R, c.G, c.B, c.A);
	p1 += gGraphicsInfo.getCameraPosition();
	p2 += gGraphicsInfo.getCameraPosition();

	SDL_RenderDrawLineF(gRenderer, p1.X, p1.Y, p2.X, p2.Y);
#endif
}

void Debug::draw(std::string color, Vec2 p1, Vec2 p2, Vec2 p3)
{
#ifdef _DEBUG
	Color c = Debug::getColor(color);
	SDL_SetRenderDrawColor(gRenderer, c.R, c.G, c.B, c.A);
	p1 += gGraphicsInfo.getCameraPosition();
	p2 += gGraphicsInfo.getCameraPosition();
	p3 += gGraphicsInfo.getCameraPosition();

	SDL_RenderDrawLineF(gRenderer, p2.X, p2.Y, p3.X, p3.Y);
	SDL_RenderDrawLineF(gRenderer, p1.X, p1.Y, p3.X, p3.Y);
	SDL_RenderDrawLineF(gRenderer, p1.X, p1.Y, p2.X, p2.Y);
#endif
}

void Debug::draw(std::string color, Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4)
{
#ifdef _DEBUG
	Color c = Debug::getColor(color);
	SDL_SetRenderDrawColor(gRenderer, c.R, c.G, c.B, c.A);
	p1 += gGraphicsInfo.getCameraPosition();
	p2 += gGraphicsInfo.getCameraPosition();
	p3 += gGraphicsInfo.getCameraPosition();
	p4 += gGraphicsInfo.getCameraPosition();


	SDL_RenderDrawLineF(gRenderer, p1.X, p1.Y, p2.X, p2.Y);
	SDL_RenderDrawLineF(gRenderer, p2.X, p2.Y, p3.X, p3.Y);
	SDL_RenderDrawLineF(gRenderer, p3.X, p3.Y, p4.X, p4.Y);
	SDL_RenderDrawLineF(gRenderer, p4.X, p4.Y, p1.X, p1.Y);
#endif
}

void Debug::draw(std::string color, SDL_Rect dest)
{
#ifdef _DEBUG
	Color c = Debug::getColor(color);
	SDL_SetRenderDrawColor(gRenderer, c.R, c.G, c.B, c.A);
	dest.x += gGraphicsInfo.getCameraPosition().X;
	dest.y += gGraphicsInfo.getCameraPosition().Y;

	SDL_RenderDrawLine(gRenderer, dest.x, dest.y, dest.x + dest.w, dest.y); //TL to TR
	SDL_RenderDrawLine(gRenderer, dest.x + dest.w, dest.y, dest.x + dest.w, dest.y + dest.h); // TR to BR
	SDL_RenderDrawLine(gRenderer, dest.x, dest.y + dest.h, dest.x + dest.w, dest.y + dest.h); // BL to BR
	SDL_RenderDrawLine(gRenderer, dest.x, dest.y, dest.x, dest.y + dest.h); //TL to BL
#endif
}
