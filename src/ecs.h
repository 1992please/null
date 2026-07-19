#pragma once 
#include "core/assert.h"

// std
#include <memory>
#include <unordered_set>
#include <unordered_map>

namespace internal
{
	struct FInternalComponentID
	{
		static int32_t GetNextComponentIDType()
		{
			static int32_t ComponentTypeID = 0;
			return ComponentTypeID++;
		}
	};
}

namespace ne
{
	typedef int32_t entity_t;

	template<typename ComponentType>
	struct ComponentTypeSquence
	{
		static int32_t value()
		{
			static const int32_t TypeValue = internal::FInternalComponentID::GetNextComponentIDType();
			return TypeValue;
		}
	};

	template<typename ElementType>
	class SimpleSparseArray
	{
	public:
		SimpleSparseArray(int numOfElements)
		{
			elements.resize(numOfElements);
			checkList.resize(numOfElements, 0);
			num = 0;
		}

		int32_t add()
		{
			int32_t index = -1;
			while (++index < checkList.size() && checkList[index]) {}
			NE_ASSERT(index < checkList.size(), "Trying to add more elements that array can handle.");
			elements[index] = ElementType();
			checkList[index] = 1;
			num++;
			return index;
		}

		void removeAt(int32_t index)
		{
			checkList[index] = 0;
			num--;
		}

		void reset()
		{
			std::fill(checkList.begin(), checkList.end(), 0);
			num = 0;
		}

		ElementType& operator[](int32_t index)
		{
			return elements[index];
		}

		class Iterator
		{
		public:
			Iterator(SimpleSparseArray& inArray, int32_t index = 0) : sparseArray(inArray)
			{
				currentIndex = index;
				while (currentIndex < sparseArray.checkList.size() && !sparseArray.checkList[currentIndex]) { currentIndex++; }
			}

			Iterator& operator++()
			{
				while (++currentIndex < sparseArray.checkList.size() && !sparseArray.checkList[currentIndex]) {}
				return *this;
			}

			ElementType& operator*() const
			{
				return sparseArray[currentIndex];
			}

			explicit operator bool() const
			{
				return currentIndex < sparseArray.checkList.size();
			}

			bool operator!=(const Iterator rhs) const { return currentIndex != rhs.currentIndex; }
			int32_t currentIndex;
			SimpleSparseArray& sparseArray;
		};

		typename Iterator begin() { return Iterator(*this); }
		typename Iterator end() { return Iterator(*this, checkList.size()); }

		std::vector<ElementType> elements;
		std::vector<uint8_t> checkList;
		int32_t num;
	};

	class ComponentPoolBase
	{
	public:
		virtual ~ComponentPoolBase() = default;

		virtual void removeIfExist(entity_t entity) = 0;

		bool has(entity_t entity) const
		{
			return entitiesArray[entity] != -1;
		}

		virtual void resetPool() = 0;

		std::unordered_set<entity_t> ownerEntities;
		std::vector<entity_t> entitiesArray;
	};

	template<typename ComponentType>
	class ComponentPool : public ComponentPoolBase
	{
	public:
		ComponentPool(int32_t numOfEntities, int32_t maxComponents) : pool(maxComponents)
		{
			ownerEntities.reserve(maxComponents);
			entitiesArray.reserve(numOfEntities);
		}
		ComponentPool(const ComponentPool&) = delete;
		ComponentPool(ComponentPool&&) = delete;


		ComponentType& add(entity_t entity)
		{
			NE_ASSERT(!has(entity), "entity already have such component.");
			const int32_t index = pool.add();
			entitiesArray[entity] = index;
			ownerEntities.insert(entity);
			return pool[index];
		}

		void remove(entity_t entity)
		{
			NE_ASSERT(has(entity), "entity don't have such component.");
			pool.removeAt(entitiesArray[entity]);

			entitiesArray[entity] = -1;
			ownerEntities.erase(entity);
		}

		ComponentType& getOrAdd(entity_t entity)
		{
			if (has(entity))
			{
				return pool[entitiesArray[entity]];
			}
			else
			{
				return add(entity);
			}
		}

		ComponentType& get(entity_t entity)
		{
			NE_ASSERT(has(entity), "entity don't have such component.");
			return pool[entitiesArray[entity]];
		}

		void reset()
		{
			pool.reset();
			std::fill(entitiesArray.begin(), entitiesArray.end(), -1);
			ownerEntities.clear();
		}

		int32_t size() const
		{
			return pool.num;
		}

		int32_t capacity() const
		{
			return pool.elements.size();
		}

		virtual void removeIfExist(entity_t entity) override
		{
			if (has(entity))
			{
				entitiesArray[entity] = -1;
				ownerEntities.erase(entity);
			}
		}

		virtual void resetPool() override
		{
			ownerEntities.clear();
			entitiesArray.clear();
			pool.reset();
		}

	public:
		SimpleSparseArray<ComponentType>::Iterator begin() { return pool.begin(); }
		SimpleSparseArray<ComponentType>::Iterator end() { return pool.end(); }

	private:
		SimpleSparseArray<ComponentType> pool;
	};

	class EntityManager
	{
	public:
		EntityManager(int32_t inExpectedNumOfEntities = 100)
			: availableEntity(-1)
		{
			reserveEntities(inExpectedNumOfEntities);
		}

		~EntityManager()
		{
			for (auto pool : pools)
			{
				pool.reset();
			}
		}

		template<typename ComponentType>
		void registerComponent(int32_t maxCapacity)
		{
			const int32_t componentTypeID = ComponentTypeSquence<ComponentType>::value();
			NE_ASSERT(pools.size() == componentTypeID, "Error in component registration.");
			pools.push_back(std::make_shared<ComponentPool<ComponentType>>(static_cast<int32_t>(entities.size()), maxCapacity));
		}

		void reserveEntities(int32_t inExpectedNumOfEntities)
		{
			entities.reserve(inExpectedNumOfEntities);
		}

		entity_t createEntity()
		{
			if (availableEntity != -1)
			{
				const entity_t Curr = availableEntity;
				availableEntity = entities[Curr];
				return entities[Curr] = Curr;
			}
			else
			{
				const entity_t newEntity = static_cast<entity_t>(entities.size());
				entities.push_back(newEntity);
				for (auto pool : pools)
				{
					pool->entitiesArray.push_back(-1);
				}
				return newEntity;
			}
		}

		void destroyEntity(entity_t entity)
		{
			NE_ASSERT(isValid(entity), "Entity id is not valid.");
			for (auto pool : pools)
			{
				pool->removeIfExist(entity);
			}
			entities[entity] = availableEntity;
			availableEntity = entity;
		}

		void reset()
		{
			availableEntity = -1;
			size_t cap = entities.capacity();
			entities.clear();
			entities.reserve(cap);
			for (auto pool : pools)
			{
				pool->resetPool();
			}
		}

		template<typename ComponentType>
		ComponentType& AddComponent(entity_t entity)
		{
			return getPool<ComponentType>().add(entity);
		}

		template<typename ComponentType>
		void RemoveComponent(entity_t entity)
		{
			return getPool<ComponentType>().remove(entity);
		}

		template<typename ComponentType>
		bool HasComponent(entity_t entity) const
		{
			return const_cast<EntityManager*>(this)->getPool<ComponentType>().has(entity);
		}

		template<typename ComponentType>
		ComponentType& GetComponent(entity_t entity)
		{
			return getPool<ComponentType>().get(entity);
		}

		void GetComponentIDs(entity_t entity, std::unordered_set<int32_t>& outComponentsID)
		{
			outComponentsID.clear();

			int32_t poolIndex = 0;
			for (auto pool : pools)
			{
				if (pool->has(entity))
					outComponentsID.insert(poolIndex);
				poolIndex++;
			}
		}

		bool isValid(entity_t entity)
		{
			return entity < entities.size() && entities[entity] == entity;
		}

		template<typename ComponentType>
		ComponentPool<ComponentType>& getPool()
		{
			return *((ComponentPool<ComponentType>*) pools[ComponentTypeSquence<ComponentType>::value()].get());
		}

		template<typename ComponentType>
		std::unordered_set<entity_t>& getEntities()
		{
			return pools[ComponentTypeSquence<ComponentType>::value()]->ownerEntities;
		}

		template<typename ComponentType>
		static int32_t getComponentStaticID()
		{
			return ComponentTypeSquence<ComponentType>::value();
		}

		size_t size() const
		{
			size_t out = entities.size();
			int32_t curr = availableEntity;
			for (; curr != -1; --out)
				curr = entities[curr];

			return out;
		}

		size_t capacity() const
		{
			return entities.size();
		}

	public:
		std::vector<int32_t>::iterator begin() { return entities.begin(); }
		std::vector<int32_t>::const_iterator begin() const { return entities.begin(); }
		std::vector<int32_t>::iterator end() { return entities.end(); }
		std::vector<int32_t>::const_iterator end()   const { return entities.end(); }
	private:
		entity_t availableEntity;
		std::vector<entity_t> entities;
		std::vector<std::shared_ptr<ComponentPoolBase>> pools;
	};
}