#pragma once

#include "core/mesh_data.h"

namespace ne::MeshUtils {

/**
 * @brief Generates a 24-vertex box with authentic face normals and [0, 1] texture coordinates per face.
 * @param size Total dimensions along X, Y, and Z axes.
 */
MeshData createBoxMeshData(const Vec3& size = Vec3(1.0f));

} // namespace ne::MeshUtils
