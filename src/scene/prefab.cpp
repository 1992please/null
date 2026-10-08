#include "scene/prefab.h"
#include "components/mesh_component.h"
#include "components/name_component.h"
#include "components/transform_component.h"
#include "core/assert.h"
#include "renderer/mesh.h"
#include "renderer/resource_manager.h"
#include "scene/transform_system.h"

namespace ne {

Prefab::Prefab(ResourceManager& ioResourceManager, const ModelData& iModelData)
    : mName(iModelData.mName), mMaterial(ioResourceManager.createMaterial()), mNodes(iModelData.mNodes) {
  mMeshes.reserve(iModelData.mMeshes.size());
  for (const MeshData& mesh : iModelData.mMeshes) {
    mMeshes.push_back(ioResourceManager.createMesh(mesh));
  }
}

Entity Prefab::instantiate(Registry& ioRegistry, const Transform& iRootTransform) const {
  Entity root = ioRegistry.createEntity();
  ioRegistry.addComponent<TransformComponent>(root, iRootTransform);
  ioRegistry.addComponent<NameComponent>(root, mName);

  std::vector<Entity> nodeEntities;
  nodeEntities.reserve(mNodes.size());

  for (const ModelNode& node : mNodes) {
    Entity entity = ioRegistry.createEntity();
    ioRegistry.addComponent<TransformComponent>(entity, node.mLocalTransform);
    ioRegistry.addComponent<NameComponent>(entity, node.mName);

    if (node.mMeshIndex >= 0) {
      NE_ASSERT(node.mMeshIndex < static_cast<int32_t>(mMeshes.size()), "Node references a mesh outside the prefab");
      const std::shared_ptr<Mesh>& mesh = mMeshes[node.mMeshIndex];
      ioRegistry.addComponent<MeshComponent>(entity, mesh, std::vector(mesh->getSubmeshes().size(), mMaterial));
    }

    NE_ASSERT(node.mParentIndex < static_cast<int32_t>(nodeEntities.size()), "Prefab nodes must list parents before children");
    TransformSystem::setParent(ioRegistry, entity, node.mParentIndex >= 0 ? nodeEntities[node.mParentIndex] : root);

    nodeEntities.push_back(entity);
  }

  return root;
}

} // namespace ne
