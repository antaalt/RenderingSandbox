#pragma once

#include "../Engine/math/color32.h"
#include "../Engine/math/colorHDR.h"
#include "../Engine/math/vec2.h"
#include "../Engine/math/vec3.h"
#include "../Engine/math/point3.h"
#include "../Engine/math/norm3.h"
#include "../Engine/math/uv2.h"
#include "../Engine/math/mat3.h"
#include "../Engine/math/mat4.h"
#include "../Engine/math/quat.h"
#include "../Engine/math/print.h"

namespace geometry {
inline math::real_t dot(const vec3 & lhs, const norm3 & rhs) { return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z; }
inline math::real_t dot(const point3 &lhs, const norm3 &rhs) { return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z; }
inline vec3 operator-(const vec3 &lhs, const norm3 &rhs) { return vec3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z); }
inline norm3 operator-(const norm3 &lhs, const vec3 &rhs) { return norm3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z); }
inline vec3 operator+(const point3 &lhs, const vec3 &rhs) { return vec3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z); }
}
namespace geo = geometry;
#include "../Engine/math/color32.inl"
#include "../Engine/math/colorHDR.inl"
#include "../Engine/math/vec2.inl"
#include "../Engine/math/vec3.inl"
#include "../Engine/math/point3.inl"
#include "../Engine/math/norm3.inl"
#include "../Engine/math/uv2.inl"
#include "../Engine/math/mat3.inl"
#include "../Engine/math/mat4.inl"
#include "../Engine/math/quat.inl"