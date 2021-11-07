#pragma once
#include <unordered_map>
struct Vec2
{
	float X, Y;
	Vec2() { X = Y = 0; }
	Vec2(float _x, float _y) { X = _x; Y = _y; }
	Vec2(int _x, int _y) { X = static_cast<float>(_x); Y = static_cast<float>(_y); }

	void operator+=(Vec2 v) { X += v.X; Y += v.Y; }
	void operator-=(Vec2 v) { X -= v.X; Y -= v.Y; }
	void operator*=(float f) { X *= f; Y *= f; }
	void operator/=(float f) { X /= f; Y /= f; }
};

inline Vec2 operator+(Vec2 a, Vec2 b) { return Vec2(a.X + b.X, a.Y + b.Y); }
inline Vec2 operator-(Vec2 a, Vec2 b) { return Vec2(a.X - b.X, a.Y - b.Y); }
inline Vec2 operator*(Vec2 v, float f) { return Vec2(v.X * f, v.Y * f); }
inline Vec2 operator*(float f, Vec2 v) { return Vec2(v.X * f, v.Y * f); }

struct Color
{
	uint8_t R, G, B, A;
	Color(uint8_t _R, uint8_t _G, uint8_t _B, uint8_t _A)
	{
		R = _R;
		G = _G;
		B = _B;
		A = _A;
	}
	Color() { R = G = B = A = 0; }
};

struct Transform
{
	Vec2 position = Vec2(0, 0);
	float rotation = 0;
	Vec2 scale = Vec2(1, 1);
};

class Timer
{
private:
	static long prevFrameTime;
	static long frameTime;
	static float time;
public:
	static void resetFrame();
	static float getTime();
	static void resetTimer();
};


struct SDL_Rect;

class Debug
{
private:
	static std::unordered_map<std::string, Color> colors;
public:
	static void registerColor(std::string, Color);

	static Color getColor(std::string);

	static void draw(std::string color, Vec2);
	static void draw(std::string color, Vec2, Vec2);
	static void draw(std::string color, Vec2, Vec2, Vec2);
	static void draw(std::string color, Vec2, Vec2, Vec2, Vec2);
	static void draw(std::string color, SDL_Rect);
};