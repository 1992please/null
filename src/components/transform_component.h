#pragma once

#include "core/math.h"

namespace ne {

struct TransformComponent {
  Vec3 mPosition{0.0f};
  Vec3 mRotation{0.0f}; // Euler angles in radians (X=pitch, Y=yaw, Z=roll)
  Vec3 mScale{1.0f};

  Mat4 getLocalMatrix() const {
    Mat4 transform = glm::translate(Mat4(1.0f), mPosition);
    transform = glm::rotate(transform, mRotation.y, Vec3(0.0f, 1.0f, 0.0f));
    transform = glm::rotate(transform, mRotation.x, Vec3(1.0f, 0.0f, 0.0f));
    transform = glm::rotate(transform, mRotation.z, Vec3(0.0f, 0.0f, 1.0f));
    return glm::scale(transform, mScale);
  }
};

} // namespace ne
