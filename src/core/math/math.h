#pragma once

/**
 * @file math.h
 * @brief Engine Math Master Include Header
 *
 * Engine Coordinate System Specification (ROS REP-103):
 *   - World Axes : +X Forward, +Y Left, +Z Up
 *   - Handedness : Right-handed coordinate system
 *                  Up = Forward x Left  (+Z = +X x +Y)
 *   - Rotations  : Positive angles are counter-clockwise about the axis (right-hand rule)
 */

#include "core/math/math_utils.h"
#include "core/math/vec2.h"
#include "core/math/vec3.h"
#include "core/math/vec4.h"
#include "core/math/mat4.h"
#include "core/math/quat.h"
