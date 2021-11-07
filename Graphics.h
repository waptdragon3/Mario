#pragma once
#include <memory>
#include <unordered_map>
#include "ECS.h"
#include "Util.h"

struct SDL_Texture;

class Texture
{
private:
	SDL_Texture* tex;
	Vec2 size;
public:
	Texture();
	void loadFromFile(std::string fileName);
	Vec2 getSize();
	//SDL_Texture* get();
	friend class TextureManager;
	friend class SpriteRenderSystem;
};

class TextureManager
{
private:
	std::unordered_map<std::string, std::shared_ptr<Texture>> strTextureMap;
public:
	std::shared_ptr<Texture> getTexture(std::string name);
	void clear();
};

struct Sprite
{
	std::shared_ptr<Texture> texture;
	Vec2 texturePos, textureSize;
};

class AnimationClip
{
public:
	//how long each frame will last in seconds
	std::vector<float> timings;
	Sprite getCurrentFrame(float currentTime);
	//will default AnimationClip::timings all to be equal to completionTime/numFrames;
	AnimationClip(std::shared_ptr<Texture> text, int numFrames, float completionTime);
	AnimationClip(std::shared_ptr<Texture> text, std::vector<float> timings);
private:
	float maxTime;
	std::shared_ptr<Texture> texture;
};

struct SpriteComponent
{
	float depth;
	//in pixels
	Vec2 renderSize = Vec2(100, 100);
	Vec2 origin;
	std::unordered_map<std::string, std::shared_ptr<AnimationClip>> animationClips;
	float timer = 0;
	std::shared_ptr<AnimationClip> currentClip;
	void setCurrentSequence(std::string);
	bool horizFlip = false;
	bool isAnimated = true;
};

//used for depth sorting
//bool operator<(SpriteComponent a, SpriteComponent b) { return a.depth < b.depth; }

class SpriteRenderSystem : public ECS::System
{
	int prevNumEntities;
public:
	void render();
};


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