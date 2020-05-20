#pragma once

#include "Geometry.h"

namespace app {

struct Camera {
	geo::mat4f transform;
	geo::degreef hFov;
	float zNear, zFar;
	float dt; // step raymarching
};

struct Sun {
	geo::vec3f direction;
};

struct Scene {
	Camera camera;
	Sun sun;
};

}