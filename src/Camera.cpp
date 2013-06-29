#include "Camera.hpp"

Camera::Camera()
{
	worldToCamera = glm::mat4(1.0);
	FOV = 45.0f; aspect = 1.0f; zNear = 0.01f; zFar = 50.0f;
	projection = glm::perspective(FOV, aspect, zNear, zFar);
}

void Camera::setPos(glm::mat4 _translation)
{
	translation = _translation;
	worldToCamera = projection * translation;
}

void Camera::setFOV(float newFOV)
{
	FOV = newFOV;
	projection = glm::perspective(FOV, aspect, zNear, zFar);
	worldToCamera = projection * translation;
}
void Camera::setAspect(float newAspect)
{
	aspect = newAspect;
	projection = glm::perspective(FOV, aspect, zNear, zFar);
	worldToCamera = projection * translation;
}
void Camera::setZNear(float newZNear)
{
	zNear = newZNear;
	projection = glm::perspective(FOV, aspect, zNear, zFar);
	worldToCamera = projection * translation;
}
void Camera::setZFar(float newZFar)
{
	zFar = newZFar;
	projection = glm::perspective(FOV, aspect, zNear, zFar);
	worldToCamera = projection * translation;
}
