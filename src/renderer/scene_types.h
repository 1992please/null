#pragma once

#include "core/math/mat4.h"
#include "core/math/vec4.h"
#include <cstdint>
#include <type_traits>
#include <volk/volk.h>

namespace ne {

/**
 * @struct GlobalUniforms
 * @brief View-projection and global frame constants passed to shaders via BDA.
 */
struct GlobalUniforms {
  Mat4 viewProj;
};

/**
 * @struct DrawInfo
 * @brief Per-mesh draw metadata referenced via Buffer Device Address (BDA) in Multi-Draw Indirect.
 */
struct DrawInfo {
  VkDeviceAddress vertices = 0;
  uint32_t instanceBaseOffset = 0;
};

/**
 * @struct InstanceData
 * @brief Per-instance transform, color tint, and bindless descriptor indices.
 */
struct InstanceData {
  Mat4 modelMatrix;
  Mat4 normalMatrix;
  Vec4 color{1.0f};
  uint32_t textureIndex = 0;
  uint32_t samplerIndex = 0;
};

/**
 * @struct ScenePushConstants
 * @brief Root 64-bit Buffer Device Address (BDA) pointers pushed to the primary scene pipeline layout.
 */
struct PushConstants {
  VkDeviceAddress drawInfos = 0;
  VkDeviceAddress globalUniforms = 0;
  VkDeviceAddress instances = 0;
};

// ABI layout verification ensuring exact binary parity with shaders/base_shader.slang & Vulkan limits
static_assert(sizeof(PushConstants) <= 128, "ScenePushConstants exceeds guaranteed Vulkan push constant size!");
static_assert(sizeof(PushConstants) == 24, "ScenePushConstants size mismatch!");
static_assert(sizeof(DrawInfo) == 16, "DrawInfo size mismatch with Slang layout!");
static_assert(sizeof(GlobalUniforms) == 64, "GlobalUniforms size mismatch!");
static_assert(sizeof(InstanceData) == 152, "InstanceData size mismatch with Slang layout!");

static_assert(std::is_standard_layout_v<PushConstants>, "ScenePushConstants must be standard layout!");
static_assert(std::is_standard_layout_v<DrawInfo>, "DrawInfo must be standard layout!");
static_assert(std::is_standard_layout_v<GlobalUniforms>, "GlobalUniforms must be standard layout!");
static_assert(std::is_standard_layout_v<InstanceData>, "InstanceData must be standard layout!");

} // namespace ne
