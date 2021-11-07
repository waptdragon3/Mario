#pragma once

#include "Math.h"
#include <array>
#include <unordered_map>
#include <cassert>
#include <bitset>
#include <queue>
#include <set>
#include <memory>

namespace ECS
{
	struct ECSException : public std::exception
	{
		const char* _what;
		ECSException(const char* msg) :_what(msg)
		{

		}
	};

	//using Entity = std::uint32_t;
	//const Entity MAX_ENTITIES = 5000;

	struct Entity
	{
		std::uint32_t ID;
		static const std::uint32_t MAX_ENTITIES = 5000;
		Entity(const std::uint32_t _id) :ID(_id) {}
	};

	bool operator==(Entity e1, Entity e2) { return e1.ID == e2.ID; }

	using ComponentType = std::uint16_t;

	const ComponentType MAX_COMPONENTS = 128;
	using Signature = std::bitset<MAX_COMPONENTS>;

	class IComponentArray
	{
	public:
		virtual void entityDestroyed(Entity entity) = 0;
	};

	template<typename T>
	class ComponentArray: public IComponentArray
	{
	private:
		std::array<T,5000> componentArray;
		std::unordered_map<Entity, int> entityToIndexMap;
		std::unordered_map<int, Entity> indexToEntityMap;
		//std::list<int> entityToIndexMap;
		int size;
	public:

		void insertData(Entity entity, T component)
		{
			if(entityToIndexMap.find(entity) != entityToIndexMap.end()) throw ECSException("Component cannot be added to the same entity twice");
			entityToIndexMap[entity] = size;
			indexToEntityMap[size] = entity;
			componentArray[size] = component;
			size++;
		}
		void removeData(Entity entity)
		{
			if (entityToIndexMap.find(entity) == entityToIndexMap.end()) throw ECSException("Entity does not have the component trying to be removed");

			int indexOfRemovedEntity = entityToIndexMap[entity];
			int indexOfLastElement = size - 1;
			//move last component to the new empty spot
			componentArray[indexOfRemovedEntity] = componentArray[indexOfLastElement];

			//update map to point to moved spot
			Entity entityOfLastElement = indexToEntityMap[indexOfLastElement];
			entityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
			indexToEntityMap[indexOfLastElement] = entityOfLastElement;

			entityToIndexMap.erase(entity);
			indexToEntityMap.erase(indexOfLastElement);

			size--;
		}
		T& getData(Entity entity)
		{
			//if (entityToIndexMap.find(entity) == entityToIndexMap.end()) throw ECSException("Entity does not have the component trying to be accessed");
			return componentArray[entityToIndexMap[entity]];
		}
		Entity getEntityFromComponent(T component)
		{
			auto found = std::find(componentArray.begin(), componentArray.end(), component);
			if(found == componentArray.end()) throw ECSException("Invalid Component");
			return indexToEntityMap[found-componentArray.begin()];
		}
		void entityDestroyed(Entity entity) override
		{
			if (entityToIndexMap.find(entity) == entityToIndexMap.end())
			{
				removeData(entity);
			}
		}
	};
	class System
	{
	public:
		std::vector<Entity> entities;
	};

	class EntityManager
	{
	private:
		std::queue<Entity> availableEntities{};
		std::array<Signature, Entity::MAX_ENTITIES> signatures;
		int livingEntityCount = 0;
	public:
		EntityManager()
		{
			for (uint32_t i = 0; i < Entity::MAX_ENTITIES; i++)
			{
				availableEntities.push(Entity(i));
			}
		}

		Entity createEntity()
		{
			if (livingEntityCount >= Entity::MAX_ENTITIES) throw ECSException("Too many entities");
			Entity e = availableEntities.front();
			availableEntities.pop();
			livingEntityCount++;
			return e;
		}

		void destroyEntity(Entity entity)
		{
			if (entity.ID >= Entity::MAX_ENTITIES) throw ECSException("Invalid entity");
			signatures[entity.ID].reset();

			availableEntities.push(entity);
			livingEntityCount--;
		}

		void setSignature(Entity entity, Signature signature)
		{
			if (entity.ID >= Entity::MAX_ENTITIES) throw ECSException("Invalid entity");
			signatures[entity.ID] = signature;
		}

		Signature getSignature(Entity entity)
		{
			if (entity.ID >= Entity::MAX_ENTITIES) throw ECSException("Invalid entity");

			return signatures[entity.ID];
		}
	};
	class ComponentManager
	{
	public:
		template<typename T>
		void registerComponent()
		{
			const char* typeName = typeid(T).name();

			componentTypes.insert({ typeName, nextComponentType });
			componentArrays.insert({ typeName, std::make_shared<ComponentArray<T>>() });

			nextComponentType++;
		}

		template<typename T>
		ComponentType getComponentType()
		{
			const char* typeName = typeid(T).name();
			return componentTypes[typeName];
		}

		template<typename T>
		void addComponent(Entity entity, T component)
		{
			getComponentArray<T>()->insertData(entity, component);
		}

		template<typename T>
		T& getComponent(Entity entity)
		{
			return getComponentArray<T>()->getData(entity);
		}

		template<typename T>
		Entity getEntityFromComponent(T component)
		{
			return getComponentArray<T>()->getEntityFromComponent(component);
		}

		void entityDestroyed(Entity entity)
		{
			for (auto const& pair : componentArrays)
			{
				auto const& component = pair.second;
				component->entityDestroyed(entity);
			}
		}

		template<typename T>
		void removeComponent(Entity entity)
		{
			getComponentArray<T>()->removeData(entity);
		}

	private:
		std::unordered_map<const char*, ComponentType> componentTypes{};
		std::unordered_map<const char*, std::shared_ptr<IComponentArray>> componentArrays;
		ComponentType nextComponentType{};
		
		template<typename T>
		std::shared_ptr<ComponentArray<T>> getComponentArray()
		{
			const char* typeName = typeid(T).name();
			return std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeName]);
		}
	};
	class SystemManager
	{
	private:
		std::unordered_map<const char*, Signature> signatures{};
		std::unordered_map<const char*, std::shared_ptr<System>> systems{};
	public:

		template<typename T>
		std::shared_ptr<T> RegisterSystem()
		{
			const char* typeName = typeid(T).name();
			if (systems.find(typeName) != systems.end()) throw ECSException("System already registered");
			auto system = std::make_shared<T>();
			systems.insert({ typeName, system });
			return system;
		}

		template<typename T>
		void setSignature(Signature signature)
		{
			const char* typeName = typeid(T).name();
			if (systems.find(typeName) == systems.end()) throw ECSException("System needs to be registered before its signature may be set");
			signatures.insert({ typeName, signature });
		}

		void entityDestroyed(Entity entity)
		{
			for (auto const& pair : systems)
			{
				auto const& system = pair.second;
				system->entities.erase(std::find(system->entities.begin(), system->entities.end(), entity));
			}
		}

		void entitySystemChanged(Entity entity, Signature entitySignature)
		{
			for (auto const& pair : systems)
			{
				auto const& systemTypeName = pair.first;
				auto const& system = pair.second;
				auto const& systemSignature = signatures[systemTypeName];
				if((entitySignature & systemSignature) == systemSignature)
				{
					auto entityIt = std::find(system->entities.begin(), system->entities.end(), entity);
					if (entityIt == system->entities.end())
						system->entities.push_back(entity);
				}
				else
				{
					auto entityIt = std::find(system->entities.begin(), system->entities.end(), entity);
					if(entityIt != system->entities.end())
						system->entities.erase(entityIt);
				}
			}
		}
	};

	class Coordinator
	{
	private:
		std::unique_ptr<EntityManager> entityManager;
		std::unique_ptr<ComponentManager> componentManager;
		std::unique_ptr<SystemManager> systemManager;
	public:
		void Init()
		{
			entityManager = std::make_unique<EntityManager>();
			componentManager = std::make_unique<ComponentManager>();
			systemManager = std::make_unique<SystemManager>();
		}
		//Entity methods
		Entity createEntity()
		{
			return entityManager->createEntity();
		}
		void destroyEntity(Entity entity)
		{
			entityManager->destroyEntity(entity);
			componentManager->entityDestroyed(entity);
			systemManager->entityDestroyed(entity);
		}

		//Component methods
		template<typename T>
		void registerComponent()
		{
			componentManager->registerComponent<T>();
		}
		template<typename T>
		void addComponent(Entity entity, T component)
		{
			componentManager->addComponent<T>(entity, component);

			auto signature = entityManager->getSignature(entity);
			signature.set(componentManager->getComponentType<T>(), true);
			entityManager->setSignature(entity, signature);

			systemManager->entitySystemChanged(entity, signature);
		}
		template<typename T>
		void removeComponent(Entity entity)
		{
			componentManager->removeComponent<T>(entity);

			auto signature = entityManager->getSignature(entity);
			signature.set(componentManager->getComponentType<T>(), false);
			entityManager->setSignature(entity, signature);

			systemManager->entitySystemChanged(entity, signature);
		}
		template<typename T>
		T& getComponent(Entity entity)
		{
			return componentManager->getComponent<T>(entity);
		}
		template<typename T>
		ComponentType getComponentType()
		{
			return componentManager->getComponentType<T>();
		}
		template<typename T>
		Entity getEntityFromComponent(T component)
		{
			return componentManager->getEntityFromComponent(component);
		}

		//System methods
		template<typename T>
		std::shared_ptr<T> registerSystem()
		{
			return systemManager->RegisterSystem<T>();
		}
		template<typename T>
		void setSystemSignature(Signature signature)
		{
			systemManager->setSignature<T>(signature);
		}

	};

} //namespace