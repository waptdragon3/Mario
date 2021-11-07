#include "PlayerController.h"
#include "Input.h"
#include "Graphics.h"
#include <iostream>
#include "Physics.h"

extern ECS::Coordinator gCoordinator;

extern std::shared_ptr<PhysicsSystem> physicsSystem;
void PlayerControllerSystem::run()
{
	for (auto e : entities)
	{
		Transform& tr = gCoordinator.getComponent<Transform>(e);
		PlayerControllerComponent& pcc = gCoordinator.getComponent<PlayerControllerComponent>(e);
		SpriteComponent& sc = gCoordinator.getComponent<SpriteComponent>(e);
		RigidBody& rb = gCoordinator.getComponent<RigidBody>(e);

		bool isCrouching = Controls::getInputDown("crouch");
		float sitCrouchingMultiplier = (isCrouching) ? pcc.crouchingMultiplier : 1;

		float horizontal = 0;
		if (Controls::getInputDown("left")) horizontal--;
		if (Controls::getInputDown("right")) horizontal++;

		//horizontal = 1;
		//isCrouching = true;

		float acceleration = horizontal * 1000;
		//float acceleration = horizontal * .001f * 500;

		acceleration *= sitCrouchingMultiplier;
		rb.velocity.X += acceleration * Timer::getTime();

		//cap speed to max
		if (rb.velocity.X > 0)
			rb.velocity.X = (rb.velocity.X > pcc.maxSpeed * sitCrouchingMultiplier) ? pcc.maxSpeed * sitCrouchingMultiplier : rb.velocity.X;
		else
			rb.velocity.X = (rb.velocity.X < -pcc.maxSpeed * sitCrouchingMultiplier) ? -pcc.maxSpeed * sitCrouchingMultiplier : rb.velocity.X;



		//slow down if not pushing any buttons
		if (horizontal == 0)
			if (rb.velocity.X > 0)
				rb.velocity.X = (rb.velocity.X < .1) ? 0 : rb.velocity.X - 1000 * Timer::getTime();
			else
				rb.velocity.X = (rb.velocity.X > -.1) ? 0 : rb.velocity.X + 1000 * Timer::getTime();

		//negate movement if too slow
		if (abs(rb.velocity.X) < 5) rb.velocity.X = 0;

		sc.horizFlip = rb.velocity.X < 0;

		//tr.position += rb.velocity * Timer::getTime();
		bool onGround = physicsSystem->rayTrace(tr.position, Vec2(0, 52));
		if (onGround)
		{
			//can only jump on the ground
			if (Controls::getInput("jump") == Controls::InputState::Pressed)
			{
				rb.velocity.Y = pcc.jumpStrength;
			}
		}

		if (!onGround)
		{
			if (rb.velocity.Y > -pcc.jumpStrength/3) sc.setCurrentSequence("jumpfall");
			else if (rb.velocity.Y < pcc.jumpStrength/3) sc.setCurrentSequence("jumprise");
			else sc.setCurrentSequence("jumpmid");
		}
		else
		{
			if (rb.velocity.X != 0)
			{
				if (isCrouching) sc.setCurrentSequence("crawl");
				else sc.setCurrentSequence("run");
			}
			else
			{
				if (isCrouching) sc.setCurrentSequence("crouch");
				else sc.setCurrentSequence("idle");
			}
		}
	}
}