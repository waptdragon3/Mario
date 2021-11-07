#pragma once
#include "ECS.h"
#include "Util.h"

struct PlayerControllerComponent
{
	float maxSpeed;
	float jumpStrength = -200;
	float crouchingMultiplier;
};

class PlayerControllerSystem : public ECS::System
{
public:
	void run();
};