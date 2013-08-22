#include "Camera.hpp"

#include "Shader.hpp"

#include <gtc/matrix_transform.hpp>
#include <iostream>

const float Camera::moveDelta = 0.4f;
const float Camera::rotDelta = 1.6f;

Camera::Camera()
	:FOV(45.0f), aspect(1.0f), zNear(0.01f), zFar(50.0f),
	 theta(0.0f), phi(0.0f),
	 rotation(glm::mat4(1.0f)), translation(glm::mat4(1.0f)),
	 mode(CENTERED)

{
	projection = glm::perspective(FOV, aspect, zNear, zFar);
	block.worldToCamera = projection * translation * rotation;
	block.cameraDir = glm::vec4(0.0, 0.0, -1.0, 1.0);
	block.cameraPos = glm::vec4(0.0, 0.0, 0.0, 1.0);

	glGenBuffers(1, &cameraBlock_ubo);
	glBindBufferRange(GL_UNIFORM_BUFFER, 
		Shader::getUBlockBindingIndex("cameraBlock"), 
		cameraBlock_ubo, 0, sizeof(block));
	glBufferData(GL_UNIFORM_BUFFER, sizeof(block), &block, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Camera::translate(const glm::vec3& t)
{
	translation = glm::translate(translation, t);
	updateBlock();
}

void Camera::fly(const glm::vec3& t)
{
	glm::vec4 translateBy = glm::vec4(glm::inverse(rotation) * glm::vec4(t, 1.0));
	translation = glm::translate(translation, 
		glm::vec3(translateBy.x, translateBy.y, translateBy.z));
	updateBlock();
}

void Camera::setPos(const glm::vec3& _pos)
{
	translation = glm::translate(glm::mat4(1.0), _pos);
	updateBlock();
}

void Camera::rotate(const float& theta, const float& phi)
{
	this->theta += theta;
	this->phi += phi;
	updateRotation();
	updateBlock();
}

void Camera::setRot(const float& theta, const float& phi)
{
	this->theta = theta;
	this->phi = phi;
	updateRotation();
	updateBlock();
}

void Camera::setFOV(const float& newFOV)
{
	FOV = newFOV;
	projection = glm::perspective(FOV, aspect, zNear, zFar);
	updateBlock();
}
void Camera::setAspect(const float& newAspect)
{
	aspect = newAspect;
	projection = glm::perspective(FOV, aspect, zNear, zFar);
	updateBlock();
}
void Camera::setZNear(const float& newZNear)
{
	zNear = newZNear;
	projection = glm::perspective(FOV, aspect, zNear, zFar);
	updateBlock();
}
void Camera::setZFar(const float& newZFar)
{
	zFar = newZFar;
	projection = glm::perspective(FOV, aspect, zNear, zFar);
	updateBlock();
}

void Camera::keyboardInput(unsigned char key, int x, int y)
{
	if(mode == CENTERED)
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
			mode = FREELOOK;
			reset();
			break;
		}
	}

	else if(mode == FREELOOK)
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
			mode = CENTERED;
			reset();
			break;
		}
	}
}

void Camera::mouseInput(int mouseX, int mouseY)
{

}

void Camera::updateRotation()
{
	//Look up/down
	rotation = glm::rotate(glm::mat4(1.0), phi, glm::vec3(1.0, 0.0, 0.0));
	//Spin around
	rotation = glm::rotate(rotation,     theta, glm::vec3(0.0, 1.0, 0.0));
}

void Camera::updateBlock()
{
	if(mode == CENTERED)
		block.worldToCamera = projection * translation * rotation;
	else if(mode == FREELOOK)
		block.worldToCamera = projection * rotation * translation;

	glm::mat4 inv = glm::inverse(block.worldToCamera);
	block.cameraPos = glm::vec4(inv[3][0], inv[3][1], inv[3][2], 1.0);

	glm::vec4 rotDir = glm::inverse(rotation) * glm::vec4(0.0, 0.0, 1.0, 1.0);
	block.cameraDir = rotDir;

	glBindBuffer(GL_UNIFORM_BUFFER, cameraBlock_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(block), &block);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Camera::reset()
{
	setRot(0.0, 0.0);
	setPos(glm::vec3(0.0, 0.0, 2.0));
}
