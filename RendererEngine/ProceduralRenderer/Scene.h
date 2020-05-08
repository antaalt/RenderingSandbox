#pragma once

#include "Geometry.h"

namespace app {

struct Camera {
	Camera() : transform(geo::mat4::identity()), hFov(60.f), zNear(0.1f), zFar(1000.f) {}
	geo::mat4 transform;
	math::Degree hFov;
	float zNear, zFar;
};

struct Scene {
	Camera camera;
};

}