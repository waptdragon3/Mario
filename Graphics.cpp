#include "Graphics.h"
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include "Util.h"

extern SDL_Renderer* gRenderer;
extern ECS::Coordinator gCoordinator;
extern bool gWireFrame;
extern GlobalInfo gGraphicsInfo;


Texture::Texture()
{
	tex = NULL;
}

void Texture::loadFromFile(std::string file)
{
	SDL_Surface* loadedSurface = IMG_Load(file.c_str());
	if (loadedSurface == NULL)
	{
		std::cerr << "Unable to load image " + file + "! SDL_image Error : " << IMG_GetError() << std::endl;
	}
	else
	{
		tex = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (tex == NULL)
		{
			std::cerr << "Unable to create texture from " + file + "! SDL_image Error : " << IMG_GetError() << std::endl;
		}
		SDL_FreeSurface(loadedSurface);

		SDL_Point s;
		SDL_QueryTexture(tex, NULL, NULL, &s.x, &s.y);
		size = Vec2((float)s.x, (float)s.y);
	}
}

Vec2 Texture::getSize()
{
	return size;
}

std::shared_ptr<Texture> TextureManager::getTexture(std::string name)
{
	auto find = strTextureMap.find(name);
	if (find != strTextureMap.end()) return find->second;

	std::shared_ptr<Texture> tex = std::make_shared<Texture>();
	tex->loadFromFile(name);
	return tex;
}

void TextureManager::clear()
{
	for (auto i : strTextureMap)
	{
		SDL_DestroyTexture(i.second->tex);
		i.second->tex = NULL;
	}
}

void SpriteRenderSystem::render()
{
	if (prevNumEntities != entities.size())
	{
		std::sort(entities.begin(), entities.end(), [](const ECS::Entity a, const ECS::Entity b)->bool
			{
				return gCoordinator.getComponent<SpriteComponent>(a).depth > gCoordinator.getComponent<SpriteComponent>(b).depth;
			});
		prevNumEntities = entities.size();
	}
	for (auto e : entities)
	{
		Transform& tr = gCoordinator.getComponent<Transform>(e);
		SpriteComponent& sc = gCoordinator.getComponent<SpriteComponent>(e);

		Sprite currentSprite = sc.currentClip->getCurrentFrame(sc.timer);

		SDL_Rect dest;
		dest.x = (int)(tr.position.X + sc.origin.X - sc.renderSize.X * .5f + gGraphicsInfo.getCameraPosition().X);
		dest.y = (int)(tr.position.Y + sc.origin.Y - sc.renderSize.Y * .5f + gGraphicsInfo.getCameraPosition().Y);
		dest.w = (int)sc.renderSize.X;
		dest.h = (int)sc.renderSize.Y;
		
		SDL_Rect src;
		src.x = (int)currentSprite.texturePos.X;
		src.y = (int)currentSprite.texturePos.Y;
		src.w = (int)currentSprite.textureSize.X;
		src.h = (int)currentSprite.textureSize.Y;

		SDL_RendererFlip flip = SDL_FLIP_NONE;
		if (sc.horizFlip) flip = SDL_FLIP_HORIZONTAL;

		SDL_RenderCopyEx(gRenderer, currentSprite.texture->tex, &src, &dest, 0, NULL, flip);

		sc.timer += Timer::getTime();
		//std::cout << Timer::getTime() << std::endl;
		if (gWireFrame)
		{
			/*SDL_SetRenderDrawColor(gRenderer, 0, 255, 0, 255);
			//Draw sprite outline
			SDL_RenderDrawLine(gRenderer, dest.x, dest.y, dest.x + dest.w, dest.y); //TL to TR
			SDL_RenderDrawLine(gRenderer, dest.x + dest.w, dest.y, dest.x + dest.w, dest.y + dest.h); // TR to BR
			SDL_RenderDrawLine(gRenderer, dest.x, dest.y + dest.h, dest.x + dest.w, dest.y + dest.h); // BL to BR
			SDL_RenderDrawLine(gRenderer, dest.x, dest.y, dest.x, dest.y + dest.h); //TL to BL
			*/
			dest.x -= (int)gGraphicsInfo.getCameraPosition().X;
			dest.y -= (int)gGraphicsInfo.getCameraPosition().Y;
			Vec2 TL, TR, BR, BL;
			TL = Vec2(dest.x, dest.y);
			TR = Vec2(dest.x + dest.w, dest.y);
			BR = Vec2(dest.x + dest.w, dest.y + dest.h);
			BL = Vec2(dest.x, dest.y + dest.h);
			Debug::draw("spriteTest", dest);

			Vec2 absoluteSpriteOrigin = sc.origin + tr.position;
			//X marks the spot for sprite origin
			Debug::draw("spriteTest", absoluteSpriteOrigin + Vec2(5, 5), absoluteSpriteOrigin + Vec2(-5, -5));
			Debug::draw("spriteTest", absoluteSpriteOrigin + Vec2(-5, 5), absoluteSpriteOrigin + Vec2(5, -5));
			//SDL_RenderDrawLine(gRenderer, (int)absoluteSpriteOrigin.X + 5, (int)absoluteSpriteOrigin.Y - 5, (int)absoluteSpriteOrigin.X - 5, (int)absoluteSpriteOrigin.Y + 5);
			//SDL_RenderDrawLine(gRenderer, (int)absoluteSpriteOrigin.X - 5, (int)absoluteSpriteOrigin.Y - 5, (int)absoluteSpriteOrigin.X + 5, (int)absoluteSpriteOrigin.Y + 5);
		}

	}
}

Sprite AnimationClip::getCurrentFrame(float currentTime)
{
	while (currentTime > maxTime)
		currentTime -= maxTime;
	int i = 0;
	for (auto t : timings)
	{
		if (currentTime < t) break;
		currentTime -= t;
		i++;
	}
	Sprite s;
	s.texture = texture;
	s.textureSize.X = texture->getSize().X / timings.size();
	s.textureSize.Y = texture->getSize().Y;
	s.texturePos.X = s.textureSize.X * i;
	s.texturePos.Y = 0;
	return s;
}

AnimationClip::AnimationClip(std::shared_ptr<Texture> text, int numFrames, float completionTime)
{
	texture = text;
	maxTime = completionTime;
	for (int i = 0; i < numFrames; i++)
		timings.push_back(completionTime / numFrames);
}

AnimationClip::AnimationClip(std::shared_ptr<Texture> text, std::vector<float> timings)
{
	texture = text;
	this->timings = timings;
	for (auto a : timings)
	{
		maxTime += a;
	}
}

void SpriteComponent::setCurrentSequence(std::string s)
{
	if (animationClips.find(s) != animationClips.end())
		currentClip = animationClips[s];
}

void GlobalInfo::setCameraPosition(Vec2 v)
{
	CameraPosition = v;
	//SDL_Rect rect;
	//rect.x = v.X;
	//rect.y = v.Y;
	//rect.w = ScreenSize.X;
	//rect.h = ScreenSize.Y;
	//SDL_RenderSetViewport(gRenderer, &rect);
}

Vec2 GlobalInfo::getCameraPosition()
{
	return CameraPosition;
}

void GlobalInfo::setCameraScale(float f)
{
	CameraScale = f;
	SDL_RenderSetScale(gRenderer, CameraScale, CameraScale);
}

float GlobalInfo::getCameraScale()
{
	return CameraScale;
}
