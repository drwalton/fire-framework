#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "Element.hpp"

#include <glm.hpp>

#include <glew.h>

class Scene;

/* Light
 * An Element which is a light source (ADT).
 * **Note** Ambient lights are not Light objects. Ambient light may only be 
 			adjusted per-scene by calling Scene::setAmbLight().
 */
class Light : public Element
{
public:
	Light(float _intensity)
		:on(1), intensity(_intensity), index(-1), scene(nullptr) {};
	virtual void setIntensity(float _intensity) = 0;
	GLuint on;
	int index;
	float intensity;
	bool shadow;
	Scene* scene;
};

/* DirLight
 * An ideal directional light source. Has only direction and intensity.
 */
class DirLight : public Light
{
public:
	DirLight(glm::vec3 _dir, float _intensity)
		:Light(_intensity), dir(_dir) {};
	void setDir(glm::vec3 _dir);
	void setIntensity(float _intensity);
	glm::vec3 dir;
};

/* PointLight
 * An ideal point light source. Has position and intensity.
 */
class PointLight : public Light
{
public:
	PointLight(glm::vec4 _pos, float _intensity)
		: Light(_intensity), pos(_pos) {};
	void setPos(glm::vec4 _pos);
	void setIntensity(float _intensity);
	glm::vec4 pos;
};

#endif
