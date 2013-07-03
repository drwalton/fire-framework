#include "Camera.hpp"

const float Camera::moveDelta = 0.4f;
const float Camera::rotDelta = 1.6f;

Camera::Camera()
	:FOV(45.0f), aspect(1.0f), zNear(0.01f), zFar(50.0f),
	 theta(0.0f), phi(0.0f),
	 rotation(glm::mat4(1.0f)), translation(glm::mat4(1.0f)),
	 mode(CameraModes::CENTERED)

{
	projection = glm::perspective(FOV, aspect, zNear, zFar);
	updateWorldToCamera();
}

void Camera::translate(const glm::vec3& t)
{
	translation = glm::translate(translation, t);
	updateWorldToCamera();
}

void Camera::fly(const glm::vec3& t)
{
	glm::vec4 translateBy = glm::vec4(glm::inverse(rotation) * glm::vec4(t, 1.0));
	translation = glm::translate(translation, glm::vec3(translateBy.x, translateBy.y, translateBy.z));
	updateWorldToCamera();
}

void Camera::setPos(const glm::vec3& _pos)
{
	translation = glm::translate(glm::mat4(1.0), _pos);
	updateWorldToCamera();
}

void Camera::rotate(const float& theta, const float& phi)
{
	this->theta += theta;
	this->phi += phi;
	updateRotation();
	updateWorldToCamera();
}

void Camera::setRot(const float& theta, const float& phi)
{
	this->theta = theta;
	this->phi = phi;
	updateRotation();
	updateWorldToCamera();
}

void Camera::setFOV(const float& newFOV)
{
	FOV = newFOV;
	projection = glm::perspective(FOV, aspect, zNear, zFar);
	updateWorldToCamera();
}
void Camera::setAspect(const float& newAspect)
{
	aspect = newAspect;
	projection = glm::perspective(FOV, aspect, zNear, zFar);
	updateWorldToCamera();
}
void Camera::setZNear(const float& newZNear)
{
	zNear = newZNear;
	projection = glm::perspective(FOV, aspect, zNear, zFar);
	updateWorldToCamera();
}
void Camera::setZFar(const float& newZFar)
{
	zFar = newZFar;
	projection = glm::perspective(FOV, aspect, zNear, zFar);
	updateWorldToCamera();
}

void Camera::keyboardInput(unsigned char key, int x, int y)
{
	if(mode == CameraModes::CENTERED)
	{
		switch(key)
		{
		case 'w':
			translate(glm::vec3(0.0, 0.0,  moveDelta));
			break;
		case 's':
			translate(glm::vec3(0.0, 0.0, -moveDelta));
			break;
		case 'j':
			rotate( rotDelta, 0.0f);
			break;
		case 'l':
			rotate(-rotDelta, 0.0f);
			break;
		case 'i':
			rotate(0.0f,  rotDelta);
			break;
		case 'k':
			rotate(0.0f, -rotDelta);
			break;
		case 'c':
			mode = CameraModes::FREELOOK;
			reset();
			break;
		}
	}

	else if(mode == CameraModes::FREELOOK)
	{
		switch(key)
		{
		case 'w':
			fly(glm::vec3(0.0, 0.0,  moveDelta));
			break;
		case 's':
			fly(glm::vec3(0.0, 0.0, -moveDelta));
			break;
		case 'a':
			fly(glm::vec3(moveDelta, 0.0, 0.0));
			break;
		case 'd':
			fly(glm::vec3(-moveDelta, 0.0, 0.0));
			break;
		case 'j':
			rotate(-rotDelta, 0.0f);
			break;
		case 'l':
			rotate( rotDelta, 0.0f);
			break;
		case 'i':
			rotate(0.0f, -rotDelta);
			break;
		case 'k':
			rotate(0.0f,  rotDelta);
			break;
		case 'c':
			mode = CameraModes::CENTERED;
			reset();
			break;
		}
	}
}

void Camera::mouseInput(int mouseX, int mouseY)
{

}

glm::vec3 Camera::getCameraDir()
{
	glm::vec4 rotDir = glm::inverse(rotation) * glm::vec4(0.0, 0.0, -1.0, 1.0);
	return glm::vec3(rotDir.x, rotDir.y, rotDir.z);
}

void Camera::updateRotation()
{
	//Look up/down
	rotation = glm::rotate(glm::mat4(1.0), phi, glm::vec3(1.0, 0.0, 0.0));
	//Spin around
	rotation = glm::rotate(rotation,     theta, glm::vec3(0.0, 1.0, 0.0));

	std::cout << theta << ", " << phi << "\n";
}

void Camera::updateWorldToCamera()
{
	if(mode == CameraModes::CENTERED)
		worldToCamera = projection * translation * rotation;
	else if(mode == CameraModes::FREELOOK)
		worldToCamera = projection * rotation * translation;
}

void Camera::reset()
{
	setRot(0.0, 0.0);
	setPos(glm::vec3(0.0, 0.0, 2.0));
}