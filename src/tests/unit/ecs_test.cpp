#include "core/ecs.h"
#include "tests/test_runner.h"

#ifndef NE_BUILD_SHIPPING

namespace ne::test {

struct PositionComponent {
  float x{0.0f}, y{0.0f}, z{0.0f};
};

struct VelocityComponent {
  float vx{0.0f}, vy{0.0f}, vz{0.0f};
};

NE_TEST_CASE("ecs", "Entity Handle Allocation & Recycling") {
  Registry registry;

  Entity e1 = registry.createEntity();
  Entity e2 = registry.createEntity();

  NE_TEST_ASSERT(registry.isValid(e1), "e1 handle must be valid.");
  NE_TEST_ASSERT(registry.isValid(e2), "e2 handle must be valid.");
  NE_TEST_ASSERT(e1 != e2, "Entity handles must be unique.");

  registry.destroyEntity(e1);
  NE_TEST_ASSERT(!registry.isValid(e1), "e1 handle must be invalid after destruction.");

  Entity e3 = registry.createEntity();
  NE_TEST_ASSERT(e3.mId == e1.mId, "e3 should recycle e1's ID index.");
  NE_TEST_ASSERT(e3.mVersion == e1.mVersion + 1, "e3 must have incremented version.");
  NE_TEST_ASSERT(!registry.isValid(e1), "e1 handle must remain invalid.");
}

NE_TEST_CASE("ecs", "Component Emplace & Sparse Access") {
  Registry registry;

  Entity e2 = registry.createEntity();
  registry.addComponent<PositionComponent>(e2, 10.0f, 20.0f, 30.0f);
  NE_TEST_ASSERT(registry.hasComponent<PositionComponent>(e2), "e2 has PositionComponent.");

  auto& pos = registry.getComponent<PositionComponent>(e2);
  NE_TEST_ASSERT(pos.x == 10.0f && pos.y == 20.0f && pos.z == 30.0f, "PositionComponent values match.");
}

NE_TEST_CASE("ecs", "Swap-and-Pop Array Integrity") {
  Registry registry;

  Entity e2 = registry.createEntity();
  Entity e3 = registry.createEntity();
  Entity e4 = registry.createEntity();

  registry.addComponent<PositionComponent>(e2, 10.0f, 20.0f, 30.0f);
  registry.addComponent<PositionComponent>(e4, 1.0f, 1.0f, 1.0f);
  registry.addComponent<PositionComponent>(e3, 5.0f, 5.0f, 5.0f);

  NE_TEST_ASSERT(registry.getPool<PositionComponent>().size() == 3, "Pool size should be 3.");
  registry.removeComponent<PositionComponent>(e2);
  NE_TEST_ASSERT(!registry.hasComponent<PositionComponent>(e2), "e2 component removed.");
  NE_TEST_ASSERT(registry.getPool<PositionComponent>().size() == 2, "Pool size should be 2.");
}

NE_TEST_CASE("ecs", "Multi-Component View Iteration") {
  Registry registry;

  Entity e3 = registry.createEntity();
  Entity e4 = registry.createEntity();

  registry.addComponent<PositionComponent>(e3, 5.0f, 5.0f, 5.0f);
  registry.addComponent<PositionComponent>(e4, 1.0f, 1.0f, 1.0f);

  registry.addComponent<VelocityComponent>(e3, 2.0f, 0.0f, 0.0f);
  registry.addComponent<VelocityComponent>(e4, 0.0f, 3.0f, 0.0f);

  int matchingCount = 0;
  registry.view<PositionComponent, VelocityComponent>().each([&](Entity e, PositionComponent& position, VelocityComponent& velocity) {
    NE_UNUSED(e);
    matchingCount++;
    position.x += velocity.vx;
    position.y += velocity.vy;
  });

  NE_TEST_ASSERT(matchingCount == 2, "View must match exactly 2 entities.");
  NE_TEST_ASSERT(registry.getComponent<PositionComponent>(e3).x == 7.0f, "e3 updated via view.");
  NE_TEST_ASSERT(registry.getComponent<PositionComponent>(e4).y == 4.0f, "e4 updated via view.");
}

NE_TEST_CASE("ecs", "Component Registration & Capacity Reservation") {
  Registry registry;
  registry.reserveComponents<PositionComponent>(64);

  auto& pool = registry.getPool<PositionComponent>();
  NE_TEST_ASSERT(pool.capacity() >= 64, "Dense component vector must reserve requested capacity.");
}

} // namespace ne::test

#endif
