#pragma once

/**
 * @file math.h
 * @brief Engine Math Master Include Header
 *
 * Engine Coordinate System Specification (Unreal Engine Style):
 *   - World Axes : +X Forward, +Y Right, +Z Up
 *   - Handedness : Left-handed coordinate system (GLM_FORCE_LEFT_HANDED)
 *                  Right = Up x Forward  (+Y = +Z x +X)
 *   - Vulkan NDC : [0.0, 1.0] Depth range (GLM_FORCE_DEPTH_ZERO_TO_ONE)
 *                  Y-axis is flipped in perspective matrix (proj[1][1] *= -1.0f)
 *                  so +Z points UP on screen.
 */

#include "math/math_utils.h"
#include "math/vec2.h"
#include "math/vec3.h"
#include "math/vec4.h"
#include "math/mat4.h"
#include "math/quat.h"
