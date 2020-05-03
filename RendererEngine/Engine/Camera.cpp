#include "Camera.h"


namespace engine {

Camera::Camera()
{
}
geom::mat4 Camera::perspective()
{
	return geom::mat4();
}
geom::mat4 Camera::orthographic()
{
	return geom::mat4();
}

}