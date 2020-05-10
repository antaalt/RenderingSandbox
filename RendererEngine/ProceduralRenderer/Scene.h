#pragma once

#include "Geometry.h"

namespace app {

struct Camera {
	Camera() : transform(geo::mat4f::identity()), hFov(60.f), zNear(0.01f), zFar(100.f), dt(1.f) {}
	geo::mat4f transform;
	geo::degreef hFov;
	float zNear, zFar;
	float dt; // step raymarching
};

struct Sun {
	Sun() : direction(geo::vec3f(0, 1, 0)) {}
	geo::vec3f direction;
};

struct Scene {
	Camera camera;
	Sun sun;
};

}