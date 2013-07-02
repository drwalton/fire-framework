#ifndef CAMERA_H
#define CAMERA_H

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <iostream>

/* Camera Modes
 * FREELOOK: Camera rotates & moves relative to itself.
 * CENTRED: Camera rotates around (0,0,0).
 */
enum CameraModes : char { FREELOOK, CENTRED };

/* Camera
 * Every Scene has a Camera object, which describes how world space is transformed into 
 * camera space. 
 * This consists of a rotation, translation and projection (applied in that order).
 */
class Camera
{
public:
	Camera();
	void translate(const glm::vec3& t);
	void fly(const glm::vec3& t);
	void setPos(const glm::vec3& _pos);
	void rotate(const float& theta, const float& phi);
	void setRot(const float& theta, const float& phi);

	void setFOV(const float& newFOV);
	void setAspect(const float& newAspect);
	void setZNear(const float& newZNear);
	void setZFar(const float& newZFar);

	void keyboardInput(unsigned char key, int x, int y);
	void mouseInput(int mouseX, int mouseY);

	glm::mat4 getMat() {return worldToCamera;};
private:
	CameraModes mode;
	glm::mat4 worldToCamera;
	glm::mat4 projection;
	glm::mat4 translation;
	glm::mat4 rotation;
	float theta;
	float phi;
	float FOV; 
	float aspect; 
	float zNear;
	float zFar;
	int lastMouseX;
	int lastMouseY;
	void updateRotation();
	void updateWorldToCamera();
	void reset();
	static const float moveDelta;
	static const float rotDelta;
};

#endif
