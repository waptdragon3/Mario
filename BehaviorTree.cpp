#include "BehaviorTree.h"

extern ECS::Coordinator gCoordinator;

void AISystem::run()
{
	for (auto e : entities)
	{
		AIComponent& aic = gCoordinator.getComponent<AIComponent>(e);
		aic.bTree.run();
	}
}