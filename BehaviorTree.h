#pragma once
#include <list>

#include "ECS.h"

//https://web.archive.org/web/20131209105717/http://www.altdevblogaday.com/2011/02/24/introduction-to-behavior-trees/


class BTNode
{
private:
public:
	enum class NodeState
	{
		Ready, Failed, Running, Success
	};
	virtual NodeState run() = 0;
	virtual void reset() {
		if (currentState == NodeState::Running) return;
		else currentState = NodeState::Ready;
	}
	virtual void attachToEntity(ECS::Entity* e) { entity = e; }
	NodeState currentState;
	ECS::Entity* entity;
	virtual ~BTNode()
	{

	}
};

//this node has multiple children
class CompositeNode : public BTNode
{
private:
	std::list<BTNode*> children;
public:
	const std::list<BTNode*>& getChildren() const { return children; }
	void addChild(BTNode* child) { children.emplace_back(child); }
	virtual void reset() override {
		BTNode::reset();
		for (BTNode* child : getChildren())
		{
			child->reset();
		}
	}

	virtual void attachToEntity(ECS::Entity* e)
	{
		BTNode::attachToEntity(e);
		for (BTNode* child : getChildren())
		{
			child->attachToEntity(e);
		}
	}

	~CompositeNode()
	{
		for (BTNode* child : getChildren())
		{
			delete child;
			child = nullptr;
		}
	}
};

//Marked as completed if one of its children succeeds
class SelectorNode : public CompositeNode
{
public:
	virtual NodeState run() override
	{
		NodeState best = NodeState::Failed;
		for (BTNode* child : getChildren())
		{
			NodeState result = child->run();
			if (result == NodeState::Success)
			{
				currentState = NodeState::Success;
				return NodeState::Success;
			}
			else if (result == NodeState::Running) best = NodeState::Running;
		}
		currentState = best;
		return best;
	}
};

//Marked as completed if all its children complete.
class SequenceNode : public CompositeNode
{
public:
	virtual NodeState run() override
	{
		for (BTNode* child : getChildren())
		{
			NodeState result = child->run();
			if (result == NodeState::Running)
			{
				currentState = NodeState::Running;
				return NodeState::Running;
			}
			else if (result == NodeState::Failed)
			{
				currentState = NodeState::Failed;
				return NodeState::Failed;
			}
		}
		currentState = NodeState::Success;
		return NodeState::Success;
	}
};

class BehaviorTree
{
private:
	BTNode* root;
public:
	void setTree(BTNode* _root) { root = _root; }
	void attachToEntity(ECS::Entity* e) { root->attachToEntity(e); }
	void run()
	{
		root->reset();
		root->run();
	}
};


struct AIComponent
{
	BehaviorTree bTree;
};

class AISystem :public ECS::System
{
public:
	void run();
};