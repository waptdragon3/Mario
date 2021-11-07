// Mario.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "ECS.h"
#include "PlayerController.h"
#include "Input.h"
#include "BehaviorTree.h"
#include "Util.h"

#include <SDL.h>
#include <iostream>
#include "Graphics.h"
#include <SDL_image.h>
#include "Physics.h"


ECS::Coordinator gCoordinator;
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TextureManager gTextureManager;
bool gWireFrame;
GlobalInfo gGraphicsInfo;
bool isPaused = true;


/*
extern ECS::Coordinator gCoordinator;
extern SDL_Window* gWindow;
extern SDL_Renderer* gRenderer;
extern TextureManager gTextureManager;
extern bool gWireFrame;
extern GlobalInfo gGraphicsInfo;
extern bool isPaused;
*/


bool SDLinit();
void close();
void registerComponents();
void registerSystems();

void addInputs();
void addEntities();

void snap();

ECS::Entity createPlayer(Vec2 position);
ECS::Entity createPlatform(Vec2 position);


std::shared_ptr<SpriteRenderSystem> spriteRenderSystem;
std::shared_ptr<PlayerControllerSystem> playerControllerSystem;
std::shared_ptr<AISystem> aiSystem;
std::shared_ptr<PhysicsSystem> physicsSystem;

int main(int argc, char** argv)
{
	SDLinit();

	gCoordinator.Init();
	registerComponents();
	registerSystems();

	addInputs();
	addEntities();


	gGraphicsInfo.setCameraScale(2);
	//gGraphicsInfo.setScreenSize(Vec2(1280, 720));
	gGraphicsInfo.setCameraPosition(Vec2(10,10));


	gWireFrame = false;
	Debug::registerColor("spriteTest", Color(0, 255, 0, 255));
	Debug::registerColor("physicsTest", Color(255, 0, 0, 255));
	Debug::registerColor("physicsTestHit", Color(0, 0, 255, 255));


	Timer::resetTimer();
	int FPSLimit = 60;
	uint32_t prevFrame = SDL_GetTicks();

	bool quit = false;
	SDL_Event e;
	while (!quit)
	{
		prevFrame = SDL_GetTicks();
		//handle events
		while (SDL_PollEvent(&e) != 0)
		{
			switch (e.type)
			{
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				Controls::proccessKeyEvent(e);
				break;
			default:
				break;
			}
		}

		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
		SDL_RenderClear(gRenderer);
		
#ifdef _DEBUG

		if (Controls::getInput("debugToggle") == Controls::InputState::Pressed) gWireFrame = !gWireFrame;

#endif // _DEBUG

		if (Controls::getInput("pause") == Controls::InputState::Pressed) isPaused = !isPaused;
		if (Controls::getInput("crouch") == Controls::InputState::Pressed) snap();


		//render stuff
		aiSystem->run();
		playerControllerSystem->run();
		physicsSystem->run();
		spriteRenderSystem->render();

		SDL_RenderPresent(gRenderer);

		Timer::resetFrame();
		Controls::resetFrame();

		int timeToWait = (int)((1000.0 / FPSLimit) - (SDL_GetTicks()- prevFrame));
		//SDL_Delay((timeToWait > 0) ? timeToWait : 0);

	}

	//SDL_Delay(2000);

	close();

	return 0;
}

void registerComponents()
{
	gCoordinator.registerComponent<Transform>();
	gCoordinator.registerComponent<SpriteComponent>();
	gCoordinator.registerComponent<PlayerControllerComponent>();
	gCoordinator.registerComponent<AIComponent>();
	gCoordinator.registerComponent<RigidBody>();
}

void registerSystems()
{
	spriteRenderSystem = gCoordinator.registerSystem<SpriteRenderSystem>();
	
	ECS::Signature spriteRenderSystemSignature;
	spriteRenderSystemSignature.set(gCoordinator.getComponentType<Transform>());
	spriteRenderSystemSignature.set(gCoordinator.getComponentType<SpriteComponent>());
	gCoordinator.setSystemSignature<SpriteRenderSystem>(spriteRenderSystemSignature);

	playerControllerSystem = gCoordinator.registerSystem<PlayerControllerSystem>();
	ECS::Signature playerControllerSystemSignature;
	playerControllerSystemSignature.set(gCoordinator.getComponentType<Transform>());
	playerControllerSystemSignature.set(gCoordinator.getComponentType<SpriteComponent>());
	playerControllerSystemSignature.set(gCoordinator.getComponentType<PlayerControllerComponent>());
	gCoordinator.setSystemSignature<PlayerControllerSystem>(playerControllerSystemSignature);

	aiSystem = gCoordinator.registerSystem<AISystem>();
	ECS::Signature aiSystemSignature;
	aiSystemSignature.set(gCoordinator.getComponentType<AIComponent>());
	aiSystemSignature.set(gCoordinator.getComponentType<Transform>());
	gCoordinator.setSystemSignature<AISystem>(aiSystemSignature);

	physicsSystem = gCoordinator.registerSystem<PhysicsSystem>();
	ECS::Signature physicsSystemSignature;
	physicsSystemSignature.set(gCoordinator.getComponentType<RigidBody>());
	physicsSystemSignature.set(gCoordinator.getComponentType<Transform>());
	gCoordinator.setSystemSignature<PhysicsSystem>(physicsSystemSignature);
}

void addInputs()
{
	Controls::addInput("left", SDLK_a);
	Controls::addInput("right", SDLK_d);
	Controls::addInput("jump", SDLK_w);
	Controls::addInput("crouch", SDLK_s);
	Controls::addInput("debugToggle", SDLK_p);
	Controls::addInput("pause", SDLK_SPACE);
}

void addEntities()
{
	/*
	createPlayer(Vec2(0, -4));
	for (int i = 0; i < 5; i++)
	{
		createPlatform(Vec2(30+64*i, 200));
	}
	createPlatform(Vec2(30 + 128, 200-32*2));
	*/

	for (int i = 0; i < ECS::Entity::MAX_ENTITIES/2; i++)
	{
		ECS::Entity e = gCoordinator.createEntity();

		Transform t;
		int x = i % 100;
		int y = i / 100;
		t.position = Vec2(x * 5, y * 5);

		SpriteComponent sc;
		sc.animationClips.insert(std::make_pair<std::string, std::shared_ptr<AnimationClip>>
			("default", std::make_shared<AnimationClip>(gTextureManager.getTexture("resources/sprites/platforms/platform2x1.png"), 1, 1.0f)));

		sc.setCurrentSequence("default");
		sc.renderSize = Vec2(4, 4);
		
		RigidBody rb;
		rb.aabb.pointA = Vec2(-2, -2);
		rb.aabb.pointB = Vec2(2, 2);
		rb.gravity = true;

		gCoordinator.addComponent<Transform>(e, t);
		gCoordinator.addComponent<SpriteComponent>(e, sc);
		if (i%11 == 0) gCoordinator.addComponent<RigidBody>(e, rb);
	}
	//createPlatform(Vec2(30+64, 200));
}

void snap()
{
	for (int i = 0; i < ECS::Entity::MAX_ENTITIES; i++)
	{
		ECS::Entity e(i);

		if (gCoordinator.hasComponent<SpriteComponent>(e))
		{

			if (rand() % 100 < 50)
			{
				//std::cout << "Entity " << e.ID << " snapped" << std::endl;
				gCoordinator.destroyEntity(e);
			}
		}
	}
}

ECS::Entity createPlayer(Vec2 position)
{
	ECS::Entity e = gCoordinator.createEntity();
	SpriteComponent sc;

	sc.animationClips.insert(std::make_pair<std::string, std::shared_ptr<AnimationClip>>
		("run", std::make_shared<AnimationClip>(gTextureManager.getTexture("resources/sprites/player/player-run.png"), 8, .1f * 8)));

	sc.animationClips.insert(std::make_pair<std::string, std::shared_ptr<AnimationClip>>
		("idle", std::make_shared<AnimationClip>(gTextureManager.getTexture("resources/sprites/player/player-idle.png"), 7, .1f * 7)));

	sc.animationClips.insert(std::make_pair<std::string, std::shared_ptr<AnimationClip>>
		("jumprise", std::make_shared<AnimationClip>(gTextureManager.getTexture("resources/sprites/player/player-jumprise.png"), 1, .1f * 1)));

	sc.animationClips.insert(std::make_pair<std::string, std::shared_ptr<AnimationClip>>
		("jumpmid", std::make_shared<AnimationClip>(gTextureManager.getTexture("resources/sprites/player/player-jumpmid.png"), 1, .1f * 1)));

	sc.animationClips.insert(std::make_pair<std::string, std::shared_ptr<AnimationClip>>
		("jumpfall", std::make_shared<AnimationClip>(gTextureManager.getTexture("resources/sprites/player/player-jumpfall.png"), 1, .1f * 1)));

	sc.animationClips.insert(std::make_pair<std::string, std::shared_ptr<AnimationClip>>
		("crouch", std::make_shared<AnimationClip>(gTextureManager.getTexture("resources/sprites/player/player-crouch.png"), 6, .1f * 6)));

	sc.animationClips.insert(std::make_pair<std::string, std::shared_ptr<AnimationClip>>
		("fflip", std::make_shared<AnimationClip>(gTextureManager.getTexture("resources/sprites/player/player-fflip.png"), 14, .1f * 14)));

	sc.animationClips.insert(std::make_pair<std::string, std::shared_ptr<AnimationClip>>
		("crawl", std::make_shared<AnimationClip>(gTextureManager.getTexture("resources/sprites/player/player-crawl.png"), 8, .1f * 8)));

	sc.setCurrentSequence("idle");
	sc.renderSize = Vec2(100, 100);

	PlayerControllerComponent pcc;
	pcc.maxSpeed = 200;
	pcc.crouchingMultiplier = .5;

	Transform t;
	t.position = position;

	RigidBody rb;
	rb.aabb.pointA = Vec2(-10, 0);
	rb.aabb.pointB = Vec2(10, 50);
	rb.gravity = true;

	rb.velocity = Vec2(0, 0);

	gCoordinator.addComponent<Transform>(e, t);
	gCoordinator.addComponent<SpriteComponent>(e, sc);
	gCoordinator.addComponent<PlayerControllerComponent>(e, pcc);
	gCoordinator.addComponent<RigidBody>(e, rb);

	return e;
}

ECS::Entity createPlatform(Vec2 position)
{
	ECS::Entity e = gCoordinator.createEntity();
	SpriteComponent sc;

	sc.animationClips.insert(std::make_pair<std::string, std::shared_ptr<AnimationClip>>
		("default", std::make_shared<AnimationClip>(gTextureManager.getTexture("resources/sprites/platforms/platform2x1.png"), 1, 1.0f)));

	sc.setCurrentSequence("default");
	sc.renderSize = Vec2(64, 32);

	Transform t;
	t.position = position;

	RigidBody rb;
	rb.aabb.pointA = Vec2(-32, -16);
	rb.aabb.pointB = Vec2(32, 16);
	rb.gravity = false;
	rb.immovable = true;

	gCoordinator.addComponent<Transform>(e, t);
	gCoordinator.addComponent<SpriteComponent>(e, sc);
	gCoordinator.addComponent<RigidBody>(e, rb);

	return e;
}

bool SDLinit()
{
	bool success = true;
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
		success = false;
	}
	else
	{
		gWindow = SDL_CreateWindow("SDL Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
			SDL_DestroyWindow(gWindow);
			success = false;
		}
		else
		{
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
				success = false;
			}
			else
			{
				SDL_SetRenderDrawColor(gRenderer, 0xff, 0xff, 0xff, 0xff);
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					std::cout << "SDL_image could not be created! SDL_imageError: " << IMG_GetError() << std::endl;
					success = false;
				}
			}
		}
	}

	return success;
}

void close()
{
	gTextureManager.clear();

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gRenderer = NULL;
	gWindow = NULL;

	IMG_Quit();
	SDL_Quit();
}

