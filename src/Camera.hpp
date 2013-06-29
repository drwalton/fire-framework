#ifndef CAMERA_H
#define CAMERA_H

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

class Camera
{
public:
	Camera();
	void setPos(glm::mat4 _translation);
	glm::mat4 getMat() {return worldToCamera;};
	void setFOV(float newFOV);
	void setAspect(float newAspect);
	void setZNear(float newZNear);
	void setZFar(float newZFar);
private:
	glm::mat4 worldToCamera;
	glm::mat4 projection;
	glm::mat4 translation;
	float FOV; 
	float aspect; 
	float zNear;
	float zFar;
};

#endif
