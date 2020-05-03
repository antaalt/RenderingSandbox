#pragma once
#include "Config.h"

namespace engine {

struct Camera
{
	Camera();

	float pnear, pfar;		// Near and far of the camera
	float fov;				// Field of view of the camera
	float aspectRatio;		// Aspect ratio of the camera
	geom::mat4 transform;	// Transform of the camera

	// Compute perspective projection matrix
	geom::mat4 perspective();
	// Compute orthographic projection matrix
	geom::mat4 orthographic();
};

}