#include "importers/gltf_importer.h"
#include "core/assert.h"
#include "core/logger.h"
#include "core/filesystem.h"

#include <cgltf/cgltf.h>

namespace ne {

ModelData GltfImporter::importModel(const std::string& iPath) {
  std::string fullPath = ne::fs::resolveContentPath(iPath);
  NE_LOG("GltfImporter: Importing model from {}", fullPath);

  cgltf_options options{};
  cgltf_data* data = nullptr;
  cgltf_result result = cgltf_parse_file(&options, fullPath.c_str(), &data);
  if (result != cgltf_result_success) {
    NE_ERROR("GltfImporter: Failed to parse glTF file {} with error {}", fullPath, static_cast<int>(result));
    return {};
  }

  result = cgltf_load_buffers(&options, data, fullPath.c_str());
  if (result != cgltf_result_success) {
    NE_ERROR("GltfImporter: Failed to load buffers for glTF file {} with error {}", fullPath, static_cast<int>(result));
    cgltf_free(data);
    return {};
  }

  ModelData model;
  model.mName = iPath;

  for (cgltf_size i = 0; i < data->meshes_count; ++i) {
    const cgltf_mesh& mesh = data->meshes[i];
    for (cgltf_size j = 0; j < mesh.primitives_count; ++j) {
      const cgltf_primitive& primitive = mesh.primitives[j];

      if (primitive.type != cgltf_primitive_type_triangles) {
        continue;
      }

      cgltf_accessor* pos_accessor = nullptr;
      cgltf_accessor* normal_accessor = nullptr;
      cgltf_accessor* texcoord_accessor = nullptr;
      cgltf_accessor* color_accessor = nullptr;

      for (cgltf_size k = 0; k < primitive.attributes_count; ++k) {
        const cgltf_attribute& attribute = primitive.attributes[k];
        if (attribute.type == cgltf_attribute_type_position) {
          pos_accessor = attribute.data;
        } else if (attribute.type == cgltf_attribute_type_normal) {
          normal_accessor = attribute.data;
        } else if (attribute.type == cgltf_attribute_type_texcoord) {
          texcoord_accessor = attribute.data;
        } else if (attribute.type == cgltf_attribute_type_color) {
          color_accessor = attribute.data;
        }
      }

      if (!pos_accessor) {
        continue;
      }

      MeshData meshData;
      meshData.mPositions.resize(pos_accessor->count);
      if (normal_accessor) {
        meshData.mNormals.resize(pos_accessor->count);
      }
      if (texcoord_accessor) {
        meshData.mTexCoords.resize(pos_accessor->count);
      }
      meshData.mColors.resize(pos_accessor->count);

      for (cgltf_size v = 0; v < pos_accessor->count; ++v) {
        cgltf_bool pos_success = cgltf_accessor_read_float(pos_accessor, v, &meshData.mPositions[v].x, 3);
        NE_ASSERT(pos_success, "Failed to read vertex position");

        if (normal_accessor) {
          cgltf_bool normal_success = cgltf_accessor_read_float(normal_accessor, v, &meshData.mNormals[v].x, 3);
          NE_ASSERT(normal_success, "Failed to read vertex normal");
        }

        if (texcoord_accessor) {
          cgltf_bool texcoord_success = cgltf_accessor_read_float(texcoord_accessor, v, &meshData.mTexCoords[v].x, 2);
          NE_ASSERT(texcoord_success, "Failed to read vertex texture coordinates");
        }

        if (color_accessor) {
          cgltf_bool color_success = cgltf_accessor_read_float(color_accessor, v, &meshData.mColors[v].x, 3);
          NE_ASSERT(color_success, "Failed to read vertex color");
        } else {
          meshData.mColors[v] = glm::vec3(1.0f);
        }
      }

      if (primitive.indices) {
        meshData.mIndices.resize(primitive.indices->count);
        for (cgltf_size idx = 0; idx < primitive.indices->count; ++idx) {
          meshData.mIndices[idx] = static_cast<uint32_t>(cgltf_accessor_read_index(primitive.indices, idx));
        }
      } else {
        meshData.mIndices.resize(pos_accessor->count);
        for (uint32_t idx = 0; idx < pos_accessor->count; ++idx) {
          meshData.mIndices[idx] = idx;
        }
      }

      if (meshData.mPositions.size() >= 3 && meshData.mIndices.size() >= 3) {
        model.mSubmeshes.push_back(std::move(meshData));
      }
    }
  }

  cgltf_free(data);
  NE_LOG("GltfImporter: Imported {} with {} submeshes", fullPath, model.mSubmeshes.size());
  return model;
}

} // namespace ne
