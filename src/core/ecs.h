#pragma once

#include "core/assert.h"
#include "core/defines.h"

// std
#include <memory>
#include <tuple>
#include <vector>

namespace ne {

/**
 * @brief Generational entity handle representing a unique entity in the ECS.
 * Combines a 32-bit entity ID (index) with a 32-bit generation version counter
 * to prevent use-after-free bugs when entity IDs are recycled.
 */
struct Entity {
  static constexpr uint32_t INVALID_ID = ~0U;

  uint32_t mId{INVALID_ID};
  uint32_t mVersion{0};

  constexpr bool isValid() const { return mId != INVALID_ID; }
  constexpr bool operator==(const Entity& iOther) const { return mId == iOther.mId && mVersion == iOther.mVersion; }
  constexpr bool operator!=(const Entity& iOther) const { return !(*this == iOther); }
};

constexpr Entity NullEntity{Entity::INVALID_ID, 0};

} // namespace ne

namespace std {
template <>
struct hash<ne::Entity> {
  size_t operator()(const ne::Entity& iEntity) const noexcept {
    return (static_cast<size_t>(iEntity.mId) << 32) | static_cast<size_t>(iEntity.mVersion);
  }
};
} // namespace std

namespace ne {

/**
 * @brief Abstract base interface for component storage pools.
 */
class IComponentPool {
public:
  virtual ~IComponentPool() = default;
  virtual void removeIfExists(Entity iEntity) = 0;
  virtual bool has(Entity iEntity) const = 0;
  virtual void clear() = 0;
};

/**
 * @brief High-performance Sparse Set component storage.
 * Maintains dense arrays for components and entities to guarantee contiguous
 * cache-line friendly memory layout and O(1) swap-and-pop removal.
 */
template <typename ComponentType>
class ComponentPool : public IComponentPool {
public:
  ComponentPool() = default;
  ComponentPool(size_t reserveComponents, size_t reserveEntities) { reserve(reserveComponents, reserveEntities); }
  ~ComponentPool() override = default;
  ComponentPool(const ComponentPool&) = delete;
  ComponentPool& operator=(const ComponentPool&) = delete;
  ComponentPool(ComponentPool&&) noexcept = default;
  ComponentPool& operator=(ComponentPool&&) noexcept = default;

  void reserve(size_t reserveComponents, size_t reserveEntities = 0) {
    mDenseComponents.reserve(reserveComponents);
    mDenseEntities.reserve(reserveComponents);
    mSparse.reserve(reserveEntities > 0 ? reserveEntities : reserveComponents);
  }

  /**
   * @brief Construct a component in-place for an entity.
   */
  template <typename... Args>
  ComponentType& emplace(Entity iEntity, Args&&... iArgs) {
    NE_ASSERT(iEntity.isValid(), "Entity handle is invalid.");
    if (iEntity.mId >= mSparse.size()) {
      mSparse.resize(iEntity.mId + 1, INVALID_INDEX);
    }

    if (mSparse[iEntity.mId] != INVALID_INDEX) {
      uint32_t denseIdx = mSparse[iEntity.mId];
      mDenseComponents[denseIdx] = ComponentType(std::forward<Args>(iArgs)...);
      return mDenseComponents[denseIdx];
    }

    uint32_t denseIdx = static_cast<uint32_t>(mDenseComponents.size());
    mSparse[iEntity.mId] = denseIdx;
    mDenseEntities.push_back(iEntity);
    mDenseComponents.emplace_back(std::forward<Args>(iArgs)...);
    return mDenseComponents.back();
  }

  ComponentType& add(Entity iEntity) { return emplace(iEntity); }
  ComponentType& add(Entity iEntity, const ComponentType& iComp) { return emplace(iEntity, iComp); }
  ComponentType& add(Entity iEntity, ComponentType&& iComp) { return emplace(iEntity, std::move(iComp)); }

  ComponentType& getOrAdd(Entity iEntity) {
    if (has(iEntity)) {
      return get(iEntity);
    }
    return add(iEntity);
  }

  /**
   * @brief Remove component associated with entity using O(1) swap-and-pop.
   */
  void remove(Entity iEntity) {
    NE_ASSERT(has(iEntity), "Entity does not have specified component.");
    uint32_t removedDenseIdx = mSparse[iEntity.mId];
    uint32_t lastDenseIdx = static_cast<uint32_t>(mDenseComponents.size()) - 1;

    if (removedDenseIdx != lastDenseIdx) {
      Entity lastEntity = mDenseEntities[lastDenseIdx];
      mDenseComponents[removedDenseIdx] = std::move(mDenseComponents[lastDenseIdx]);
      mDenseEntities[removedDenseIdx] = lastEntity;
      mSparse[lastEntity.mId] = removedDenseIdx;
    }

    mDenseComponents.pop_back();
    mDenseEntities.pop_back();
    mSparse[iEntity.mId] = INVALID_INDEX;
  }

  void removeIfExists(Entity iEntity) override {
    if (has(iEntity)) {
      remove(iEntity);
    }
  }

  bool has(Entity iEntity) const override {
    return iEntity.isValid() && iEntity.mId < mSparse.size() && mSparse[iEntity.mId] != INVALID_INDEX;
  }

  ComponentType& get(Entity iEntity) {
    NE_ASSERT(has(iEntity), "Entity does not have specified component.");
    return mDenseComponents[mSparse[iEntity.mId]];
  }

  const ComponentType& get(Entity iEntity) const {
    NE_ASSERT(has(iEntity), "Entity does not have specified component.");
    return mDenseComponents[mSparse[iEntity.mId]];
  }

  void clear() override {
    mDenseComponents.clear();
    mDenseEntities.clear();
    mSparse.clear();
  }

  void reset() { clear(); }
  void resetPool() { clear(); }

  size_t size() const { return mDenseComponents.size(); }
  size_t capacity() const { return mDenseComponents.capacity(); }

  ComponentType* data() { return mDenseComponents.data(); }
  const ComponentType* data() const { return mDenseComponents.data(); }

  const std::vector<Entity>& entities() const { return mDenseEntities; }
  const std::vector<ComponentType>& components() const { return mDenseComponents; }

  // Range-based iteration support over contiguous dense components
  auto begin() { return mDenseComponents.begin(); }
  auto end() { return mDenseComponents.end(); }
  auto begin() const { return mDenseComponents.cbegin(); }
  auto end() const { return mDenseComponents.cend(); }

private:
  static constexpr uint32_t INVALID_INDEX = ~0U;
  std::vector<uint32_t> mSparse;
  std::vector<Entity> mDenseEntities;
  std::vector<ComponentType> mDenseComponents;
};

/**
 * @brief Zero-allocation multi-component view for iterating over entities
 * matching a set of required component types.
 */
template <typename... Components>
class View {
public:
  View(ComponentPool<Components>&... iPools) : mPools(std::forward_as_tuple(iPools...)) {}

  /**
   * @brief Iterate matching entities. Callback can accept (Entity, Comp1&, Comp2&...)
   * or (Comp1&, Comp2&...).
   */
  template <typename Func>
  void each(Func&& iFunc) {
    const std::vector<Entity>* smallestEntities = getSmallestEntities();
    if (!smallestEntities)
      return;

    for (Entity entity : *smallestEntities) {
      if ((std::get<ComponentPool<Components>&>(mPools).has(entity) && ...)) {
        if constexpr (std::is_invocable_v<Func, Entity, Components&...>) {
          iFunc(entity, std::get<ComponentPool<Components>&>(mPools).get(entity)...);
        } else {
          iFunc(std::get<ComponentPool<Components>&>(mPools).get(entity)...);
        }
      }
    }
  }

private:
  const std::vector<Entity>* getSmallestEntities() const {
    const std::vector<Entity>* smallest = nullptr;
    size_t minSize = SIZE_MAX;

    std::apply(
        [&](const auto&... pool) {
          auto checkPool = [&](const auto& p) {
            if (p.size() < minSize) {
              minSize = p.size();
              smallest = &p.entities();
            }
          };
          (checkPool(pool), ...);
        },
        mPools);

    return smallest;
  }

  std::tuple<ComponentPool<Components>&...> mPools;
};

/**
 * @brief Central manager for creating entities, attaching components, and managing system views.
 */
class Registry {
public:
  Registry(int32_t inExpectedNumOfEntities = 100) { reserveEntities(inExpectedNumOfEntities); }

  ~Registry() = default;
  Registry(const Registry&) = delete;
  Registry& operator=(const Registry&) = delete;
  Registry(Registry&&) noexcept = default;
  Registry& operator=(Registry&&) noexcept = default;

  void reserveEntities(int32_t count) { mGenerations.reserve(count); }

  /**
   * @brief Create a new entity with a unique generational handle.
   */
  Entity createEntity() {
    uint32_t id;
    if (!mFreeEntities.empty()) {
      id = mFreeEntities.back();
      mFreeEntities.pop_back();
    } else {
      id = static_cast<uint32_t>(mGenerations.size());
      mGenerations.push_back(0);
    }
    return Entity{id, mGenerations[id]};
  }

  /**
   * @brief Destroy entity, invalidate its handles, and remove all attached components.
   */
  void destroyEntity(Entity iEntity) {
    if (!isValid(iEntity))
      return;

    for (auto& pool : mPools) {
      if (pool) {
        pool->removeIfExists(iEntity);
      }
    }

    mGenerations[iEntity.mId]++;
    mFreeEntities.push_back(iEntity.mId);
  }

  /**
   * @brief Check if an entity handle is valid and active.
   */
  bool isValid(Entity iEntity) const {
    return iEntity.isValid() && iEntity.mId < mGenerations.size() && mGenerations[iEntity.mId] == iEntity.mVersion;
  }

  template <typename ComponentType>
  void reserveComponents(size_t reserveComponents = 0) {
    getPool<ComponentType>().reserve(reserveComponents, mGenerations.capacity());
  }

  template <typename ComponentType, typename... Args>
  ComponentType& addComponent(Entity iEntity, Args&&... iArgs) {
    return getPool<ComponentType>().emplace(iEntity, std::forward<Args>(iArgs)...);
  }

  template <typename ComponentType>
  void removeComponent(Entity iEntity) {
    getPool<ComponentType>().remove(iEntity);
  }

  template <typename ComponentType>
  bool hasComponent(Entity iEntity) const {
    const auto* pool = getPoolIfExists<ComponentType>();
    return pool && pool->has(iEntity);
  }

  template <typename ComponentType>
  ComponentType& getComponent(Entity iEntity) {
    return getPool<ComponentType>().get(iEntity);
  }

  template <typename ComponentType>
  const ComponentType& getComponent(Entity iEntity) const {
    const auto* pool = getPoolIfExists<ComponentType>();
    NE_ASSERT(pool != nullptr, "Component pool does not exist.");
    return pool->get(iEntity);
  }

  template <typename... Components>
  View<Components...> view() {
    return View<Components...>(getPool<Components>()...);
  }

  template <typename ComponentType>
  ComponentPool<ComponentType>& getPool() {
    uint32_t typeId = getComponentTypeID<ComponentType>();
    if (typeId >= mPools.size()) {
      mPools.resize(typeId + 1);
    }
    if (!mPools[typeId]) {
      mPools[typeId] = std::make_unique<ComponentPool<ComponentType>>(0, mGenerations.capacity());
    }
    return *static_cast<ComponentPool<ComponentType>*>(mPools[typeId].get());
  }

  template <typename ComponentType>
  const ComponentPool<ComponentType>* getPoolIfExists() const {
    uint32_t typeId = getComponentTypeID<ComponentType>();
    if (typeId < mPools.size() && mPools[typeId]) {
      return static_cast<const ComponentPool<ComponentType>*>(mPools[typeId].get());
    }
    return nullptr;
  }

  void reset() {
    for (auto& pool : mPools) {
      if (pool) {
        pool->clear();
      }
    }
    mGenerations.clear();
    mFreeEntities.clear();
  }

  size_t size() const { return mGenerations.size() - mFreeEntities.size(); }
  size_t capacity() const { return mGenerations.capacity(); }

private:
  template <typename T>
  static uint32_t getComponentTypeID() {
    static uint32_t id = sNextComponentID++;
    return id;
  }

  static inline uint32_t sNextComponentID = 0;

  std::vector<uint32_t> mGenerations;
  std::vector<uint32_t> mFreeEntities;
  std::vector<std::unique_ptr<IComponentPool>> mPools;
};

} // namespace ne