#pragma once
#include "Util.h"
#include "ECS.h"
//TODO: 4 Physics System

struct AABB
{
	Vec2 pointA, pointB;
};
AABB operator+(AABB a, Vec2 b);

struct RigidBody
{
	AABB aabb;
	Vec2 velocity;
	bool gravity = false;
	bool immovable = false;
	bool slideOnCollide = true;
};

class PhysicsSystem :public ECS::System
{
public:
	float physicsScale = 5;
	Vec2 gravity = Vec2(0, 400);
	int physicsTicksPerSecond = 1;
	void run();
	bool rayTrace(Vec2 origin, Vec2 direction);
};