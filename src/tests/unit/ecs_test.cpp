#ifndef NE_BUILD_SHIPPING

#include "tests/test_runner.h"
#include "core/ecs.h"
#include "components/camera_component.h"
#include "components/transform_component.h"
#include "components/mesh_component.h"

namespace ne::test {

struct PositionComponent {
  float x{0.0f}, y{0.0f}, z{0.0f};
};

struct VelocityComponent {
  float vx{0.0f}, vy{0.0f}, vz{0.0f};
};

struct TagComponent {
  int tag{0};
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
  NE_TEST_ASSERT(math::equals(pos.x, 10.0f) && math::equals(pos.y, 20.0f) && math::equals(pos.z, 30.0f), "PositionComponent values match.");
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

NE_TEST_CASE("ecs", "Cascading Entity Destruction & Multi-Pool Cleanup") {
  Registry registry;

  Entity e1 = registry.createEntity();
  registry.addComponent<PositionComponent>(e1, 1.0f, 2.0f, 3.0f);
  registry.addComponent<VelocityComponent>(e1, 0.1f, 0.2f, 0.3f);
  registry.addComponent<TagComponent>(e1, 100);

  Entity e2 = registry.createEntity();
  registry.addComponent<PositionComponent>(e2, 4.0f, 5.0f, 6.0f);

  NE_TEST_ASSERT(registry.getPool<PositionComponent>().size() == 2, "Position pool size is 2.");
  NE_TEST_ASSERT(registry.getPool<VelocityComponent>().size() == 1, "Velocity pool size is 1.");
  NE_TEST_ASSERT(registry.getPool<TagComponent>().size() == 1, "Tag pool size is 1.");

  registry.destroyEntity(e1);

  NE_TEST_ASSERT(!registry.isValid(e1), "e1 is invalidated.");
  NE_TEST_ASSERT(!registry.hasComponent<PositionComponent>(e1), "e1 position removed.");
  NE_TEST_ASSERT(!registry.hasComponent<VelocityComponent>(e1), "e1 velocity removed.");
  NE_TEST_ASSERT(!registry.hasComponent<TagComponent>(e1), "e1 tag removed.");

  NE_TEST_ASSERT(registry.getPool<PositionComponent>().size() == 1, "Position pool decremented to 1.");
  NE_TEST_ASSERT(registry.getPool<VelocityComponent>().size() == 0, "Velocity pool decremented to 0.");
  NE_TEST_ASSERT(registry.getPool<TagComponent>().size() == 0, "Tag pool decremented to 0.");
  NE_TEST_ASSERT(registry.hasComponent<PositionComponent>(e2), "e2 still has PositionComponent.");
}

NE_TEST_CASE("ecs", "getOrAdd Behavior") {
  Registry registry;
  auto& pool = registry.getPool<PositionComponent>();

  Entity e = registry.createEntity();
  NE_TEST_ASSERT(!pool.has(e), "e starts without component.");

  // First call adds component
  auto& comp1 = pool.getOrAdd(e);
  comp1.x = 42.0f;
  NE_TEST_ASSERT(pool.has(e), "Component was added by getOrAdd.");

  // Second call retrieves existing component
  auto& comp2 = pool.getOrAdd(e);
  NE_TEST_ASSERT(math::equals(comp2.x, 42.0f), "getOrAdd returns existing component reference.");
  NE_TEST_ASSERT(&comp1 == &comp2, "getOrAdd returns same memory address.");
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
  NE_TEST_ASSERT(math::equals(registry.getComponent<PositionComponent>(e3).x, 7.0f), "e3 updated via view.");
  NE_TEST_ASSERT(math::equals(registry.getComponent<PositionComponent>(e4).y, 4.0f), "e4 updated via view.");
}

NE_TEST_CASE("ecs", "Disjoint & Empty View Queries") {
  Registry registry;

  Entity e1 = registry.createEntity();
  registry.addComponent<PositionComponent>(e1, 1.0f, 2.0f, 3.0f);

  Entity e2 = registry.createEntity();
  registry.addComponent<VelocityComponent>(e2, 4.0f, 5.0f, 6.0f);

  int count = 0;
  // Neither e1 nor e2 has BOTH Position and Velocity
  registry.view<PositionComponent, VelocityComponent>().each([&](Entity, PositionComponent&, VelocityComponent&) {
    count++;
  });

  NE_TEST_ASSERT(count == 0, "Disjoint multi-component view should match 0 entities.");
}

NE_TEST_CASE("ecs", "Dense Component Pool Iteration") {
  Registry registry;
  Entity e1 = registry.createEntity();
  Entity e2 = registry.createEntity();

  registry.addComponent<TagComponent>(e1, 10);
  registry.addComponent<TagComponent>(e2, 20);

  auto& pool = registry.getPool<TagComponent>();
  int sum = 0;
  for (const auto& tagComp : pool.components()) {
    sum += tagComp.tag;
  }
  NE_TEST_ASSERT(sum == 30, "Range-based for loop over pool.components() produces correct sum.");
  NE_TEST_ASSERT(pool.data() != nullptr, "pool.data() returns valid pointer.");
}

NE_TEST_CASE("ecs", "Registry Reset") {
  Registry registry;
  Entity e1 = registry.createEntity();
  Entity e2 = registry.createEntity();
  registry.addComponent<PositionComponent>(e1, 1.0f, 1.0f, 1.0f);
  registry.addComponent<PositionComponent>(e2, 2.0f, 2.0f, 2.0f);

  NE_TEST_ASSERT(registry.size() == 2, "Registry size before reset is 2.");
  registry.reset();

  NE_TEST_ASSERT(registry.size() == 0, "Registry size after reset is 0.");
  NE_TEST_ASSERT(!registry.isValid(e1), "e1 invalid after reset.");
  NE_TEST_ASSERT(!registry.isValid(e2), "e2 invalid after reset.");
  NE_TEST_ASSERT(registry.getPool<PositionComponent>().size() == 0, "Position pool cleared after reset.");
}

NE_TEST_CASE("ecs", "Camera and Mesh Component View Queries") {
  Registry registry;

  Entity cam = registry.createEntity();
  registry.addComponent<TransformComponent>(cam, Vec3(-5.0f, 0.0f, 1.0f));
  auto& camComp = registry.addComponent<CameraComponent>(cam);
  camComp.mIsPrimary = true;

  Entity meshObj = registry.createEntity();
  registry.addComponent<TransformComponent>(meshObj, Vec3(0.0f, 2.0f, 0.0f));
  registry.addComponent<MeshComponent>(meshObj);

  Mat4 resolvedViewProj{1.0f};
  int camCount = 0;
  registry.view<TransformComponent, CameraComponent>().each([&](Entity e, const TransformComponent& t, const CameraComponent& c) {
    NE_UNUSED(e);
    if (c.mIsPrimary) {
      resolvedViewProj = c.getViewProjectionMatrix(t);
      camCount++;
    }
  });

  NE_TEST_ASSERT(camCount == 1, "Exactly one primary camera entity resolved.");
  NE_TEST_ASSERT(!resolvedViewProj.equals(Mat4::Identity), "Resolved ViewProjection matrix must not be Identity.");

  int meshCount = 0;
  registry.view<TransformComponent, MeshComponent>().each([&](Entity e, const TransformComponent& t, const MeshComponent& m) {
    NE_UNUSED(e);
    NE_UNUSED(m);
    NE_TEST_ASSERT(t.getPosition().equals(Vec3(0.0f, 2.0f, 0.0f)), "Mesh entity transform position matches.");
    meshCount++;
  });

  NE_TEST_ASSERT(meshCount == 1, "Exactly one mesh entity matched in view.");
}

NE_TEST_CASE("ecs", "In-Place Component Constructor Overload Dispatch") {
  Registry registry;

  // 1. TransformComponent with (pos, rot, scale)
  Entity e1 = registry.createEntity();
  Quat rot = Quat::angleAxis(math::radians(45.0f), Vec3::Up);
  auto& transform = registry.addComponent<TransformComponent>(e1, Vec3(1.0f, 2.0f, 3.0f), rot, Vec3(2.0f, 2.0f, 2.0f));

  NE_TEST_ASSERT(transform.getPosition().equals(Vec3(1.0f, 2.0f, 3.0f)), "Position forwarded correctly.");
  NE_TEST_ASSERT(transform.getRotation().equals(rot), "Rotation forwarded correctly.");
  NE_TEST_ASSERT(transform.getScale().equals(Vec3(2.0f, 2.0f, 2.0f)), "Scale forwarded correctly.");

  // 2. CameraComponent with (fov, aspect, near, far, reverseZ)
  Entity e2 = registry.createEntity();
  auto& camera = registry.addComponent<CameraComponent>(e2, 60.0f, 16.0f / 9.0f, 0.5f, 500.0f, false);
  NE_TEST_ASSERT(math::equals(camera.mFovDeg, 60.0f), "Camera FOV forwarded correctly.");
  NE_TEST_ASSERT(!camera.mUseReverseZ, "Camera reverse-Z flag forwarded correctly.");
  NE_TEST_ASSERT(!camera.mProjectionMatrix.equals(Mat4::Identity), "Projection matrix initialized immediately on construction.");
}

} // namespace ne::test

#endif
