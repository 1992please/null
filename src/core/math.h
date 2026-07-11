#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ne::math {

// Vulkan-native perspective matrix (applies Y-flip correction automatically)
inline glm::mat4 perspective(float fovYRad, float aspect, float nearVal, float farVal) {
    glm::mat4 proj = glm::perspective(fovYRad, aspect, nearVal, farVal);
    proj[1][1] *= -1.0f; // Vulkan NDC has Y pointing down, whereas GLM assumes Y pointing up
    return proj;
}

// Camera view matrix construction wrapper
inline glm::mat4 lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up) {
    return glm::lookAt(eye, center, up);
}

} // namespace ne::math
