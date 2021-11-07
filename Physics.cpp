#include "Physics.h"
#include "Util.h"
#include "Graphics.h"
#include <iostream>


extern ECS::Coordinator gCoordinator;
extern bool gWireFrame;
extern GlobalInfo gGraphicsInfo;


bool isColliding(AABB aabb1, Vec2 pos1, AABB aabb2, Vec2 pos2)
{
	return (aabb1.pointA+pos1).X < (pos2 + aabb2.pointB).X &&
		(aabb1.pointB + pos1).X > (pos2 + aabb2.pointA).X &&
		(aabb1.pointA+pos1).Y < (pos2 + aabb2.pointB).Y &&
		(aabb1.pointB + pos1).Y > (pos2 + aabb2.pointA).Y;
}

Vec2 calculateAABBDistanceTo(AABB a1,Vec2 pos1, AABB a2, Vec2 pos2)
{
	Vec2 ret;
	AABB e1, e2;
	e1.pointA = a1.pointA + pos1;
	e1.pointB = a1.pointB + pos1;
	e2.pointA = a2.pointA + pos2;
	e2.pointB = a2.pointB + pos2;

	if (e1.pointA.X < e2.pointA.X)
	{
		ret.X = e2.pointA.X - (e1.pointB.X);
	}
	else if (e1.pointA.X > e2.pointA.X)
	{
		ret.X = e1.pointA.X - (e2.pointB.X);
	}

	if (e1.pointA.Y < e2.pointA.Y)
	{
		ret.Y = e2.pointA.Y - (e1.pointB.Y);
	}
	else if (e1.pointA.Y > e2.pointA.Y)
	{
		ret.Y = e1.pointA.Y - (e2.pointB.Y);
	}

	return ret;
}

void PhysicsSystem::run()
{
	float dtime = Timer::getTime() / physicsTicksPerSecond;
	for (int iteration = 0; iteration < physicsTicksPerSecond; iteration++)
	{
		for (ECS::Entity e : entities)
		{
			Transform& tr = gCoordinator.getComponent<Transform>(e);
			RigidBody& rb = gCoordinator.getComponent<RigidBody>(e);

			if (rb.immovable) continue;

			if (rb.gravity)
			{
				rb.velocity += gravity * dtime;
			}

			Vec2 intendedMovement = rb.velocity * dtime;
			Vec2 pointA = rb.aabb.pointA + tr.position;
			Vec2 pointB = rb.aabb.pointB + tr.position;
			bool hit = false;

			float shortestTime = 0;

			//Debug::draw("physicsTestHit", tr.position, tr.position + .1*rb.velocity);

			for (ECS::Entity e1 : entities)
			{
				if (e == e1) continue;
				Transform& tr1 = gCoordinator.getComponent<Transform>(e1);
				RigidBody& rb1 = gCoordinator.getComponent<RigidBody>(e1);

				if (isColliding(rb.aabb, tr.position + rb.velocity * dtime, rb1.aabb, tr1.position + rb1.velocity * dtime))
				{
					hit = true;
					/*
					Vec2 UR = tr.position + gGraphicsInfo.getCameraPosition() + rb.aabb.pointA;
					Vec2 UL = tr.position + gGraphicsInfo.getCameraPosition() + Vec2(rb.aabb.pointA.X, rb.aabb.pointB.Y);
					Vec2 BL = tr.position + gGraphicsInfo.getCameraPosition() + rb.aabb.pointB;
					Vec2 BR = tr.position + gGraphicsInfo.getCameraPosition() + Vec2(rb.aabb.pointB.X, rb.aabb.pointA.Y);
					Debug::draw("physicsTest", UR, UL, BL, BR);
					*/


					Vec2 distance = calculateAABBDistanceTo(rb.aabb, tr.position, rb1.aabb, tr1.position);
					Vec2 timetoCollide;
					timetoCollide.X = rb.velocity.X != 0 ? abs(distance.X / rb.velocity.X) : 0;
					timetoCollide.Y = rb.velocity.Y != 0 ? abs(distance.Y / rb.velocity.Y) : 0;
					if (rb.velocity.X != 0 && rb.velocity.Y == 0)
					{
						shortestTime = std::min(timetoCollide.X, dtime);
						intendedMovement.X = shortestTime * rb.velocity.X;
					}
					else if (rb.velocity.Y != 0 && rb.velocity.X == 0)
					{
						shortestTime = std::min(timetoCollide.Y, dtime);
						intendedMovement.Y = shortestTime * rb.velocity.Y;
					}
					else
					{
						shortestTime = std::min(abs(timetoCollide.X), abs(timetoCollide.Y));
						shortestTime = std::min(shortestTime, dtime);
						intendedMovement = shortestTime * rb.velocity;
					}
					if (shortestTime > dtime) shortestTime = dtime;
					if (rb.slideOnCollide)
					{
						if (shortestTime == timetoCollide.X)
						{
							intendedMovement.X = 0;
							intendedMovement.Y = rb.velocity.Y * dtime;
							rb.velocity.X = 0;
						}
						if (shortestTime == timetoCollide.Y)
						{
							intendedMovement.Y = 0;
							intendedMovement.X = rb.velocity.X * dtime;
							rb.velocity.Y = 0;
						}
					}
				}
			}

			//intendedMovement = shortestTime * rb.velocity;

			tr.position += intendedMovement;

			//Debug::draw("physicsTest", tr.position, tr.position+10*intendedMovement);

			if (gWireFrame)
			{
				Vec2 UR, UL, BL, BR;
				UR = tr.position + rb.aabb.pointA;
				UL = tr.position + Vec2(rb.aabb.pointA.X, rb.aabb.pointB.Y);
				BL = tr.position + rb.aabb.pointB;
				BR = tr.position + Vec2(rb.aabb.pointB.X, rb.aabb.pointA.Y);
				if (hit) Debug::draw("physicsTestHit", UR, UL, BL, BR);
				else Debug::draw("physicsTest", UR, UL, BL, BR);
			}
		}
	}
}

bool PhysicsSystem::rayTrace(Vec2 origin, Vec2 direction)
{
	Vec2 point = origin + direction;
	for (ECS::Entity e : entities)
	{
		Transform& tr = gCoordinator.getComponent<Transform>(e);
		RigidBody& rb = gCoordinator.getComponent<RigidBody>(e);

		AABB globalAABB = rb.aabb;
		globalAABB.pointA += tr.position;
		globalAABB.pointB += tr.position;

		if (point.X > globalAABB.pointA.X && point.X < globalAABB.pointB.X)
		{
			if (point.Y > globalAABB.pointA.Y && point.Y < globalAABB.pointB.Y)
			{
				return true;
			}
		}
	}

	return false;
}

AABB operator+(AABB a, Vec2 b)
{
	AABB aabb;
	aabb.pointA = a.pointA + b;
	aabb.pointB = a.pointB + b;
	return aabb;
}
