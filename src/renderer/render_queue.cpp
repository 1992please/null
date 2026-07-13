#include "renderer/render_queue.h"
#include "renderer/mesh.h"
#include "renderer/renderer.h"
#include "renderer/pipeline.h"
#include "renderer/buffer.h"

namespace ne {

struct DrawInfo {
  VkDeviceAddress vertices;
  uint32_t instanceBaseOffset;
  uint32_t padding;
};

struct PushConstants {
  VkDeviceAddress drawInfos;
  VkDeviceAddress globalUniforms;
  VkDeviceAddress instances;
};

void RenderQueue::add(Mesh* mesh, const glm::mat4& transform, const glm::vec4& color) {
  mDrawBatches[mesh].push_back({transform, color});
}

void RenderQueue::clear() {
  mDrawBatches.clear();
}

void RenderQueue::submit(VkCommandBuffer cmd, Renderer* renderer, Pipeline* pipeline, VkDeviceAddress globalUniformsAddr) {
  if (mDrawBatches.empty()) return;

  uint32_t numUniqueMeshes = static_cast<uint32_t>(mDrawBatches.size());

  std::vector<DrawInfo> drawInfos;
  drawInfos.reserve(numUniqueMeshes);

  std::vector<InstanceData> instanceData;
  std::vector<VkDrawIndexedIndirectCommand> indirectCommands;
  indirectCommands.reserve(numUniqueMeshes);

  uint32_t currentBaseInstance = 0;

  for (auto& [mesh, instances] : mDrawBatches) {
    uint32_t instanceCount = static_cast<uint32_t>(instances.size());

    // 1. Populate DrawInfo metadata for this mesh
    DrawInfo drawInfo{};
    drawInfo.vertices = mesh->getVertexBufferAddress();
    drawInfo.instanceBaseOffset = currentBaseInstance;
    drawInfos.push_back(drawInfo);

    // 2. Append all instances
    for (const auto& inst : instances) {
      instanceData.push_back({inst.transform, inst.color});
    }

    // 3. Populate indirect draw command
    VkDrawIndexedIndirectCommand indirectCmd{};
    indirectCmd.indexCount = mesh->getIndexCount();
    indirectCmd.instanceCount = instanceCount;
    indirectCmd.firstIndex = mesh->getFirstIndex();
    indirectCmd.vertexOffset = 0;
    indirectCmd.firstInstance = 0;
    indirectCommands.push_back(indirectCmd);

    currentBaseInstance += instanceCount;
  }

  // 4. Upload to current frame's host-mapped GPU buffer
  Buffer* uploadBuffer = renderer->getUploadBuffer();
  VkDeviceAddress drawInfosAddr = uploadBuffer->upload(drawInfos.data(), drawInfos.size() * sizeof(DrawInfo));
  VkDeviceAddress instancesAddr = uploadBuffer->upload(instanceData.data(), instanceData.size() * sizeof(InstanceData));
  VkDeviceAddress indirectCmdsAddr = uploadBuffer->upload(indirectCommands.data(), indirectCommands.size() * sizeof(VkDrawIndexedIndirectCommand));

  VkDeviceSize indirectOffset = indirectCmdsAddr - uploadBuffer->getDeviceAddress();

  // 5. Submit the draw using BDA push constants
  PushConstants pc{};
  pc.drawInfos = drawInfosAddr;
  pc.globalUniforms = globalUniformsAddr;
  pc.instances = instancesAddr;

  vkCmdPushConstants(cmd, pipeline->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &pc);

  vkCmdDrawIndexedIndirect(cmd, uploadBuffer->getBuffer(), indirectOffset,
                           numUniqueMeshes, sizeof(VkDrawIndexedIndirectCommand));
}

} // namespace ne
