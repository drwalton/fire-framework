#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "Element.hpp"

#include <glm.hpp>

#include <glew.h>

class Scene;

class Light : public Element
{
public:
	Light(float _intensity, bool _shadow)
		:on(1), intensity(_intensity), shadow(_shadow), index(-1), scene(nullptr) {};
	virtual void setIntensity(float _intensity) = 0;
	GLuint on;
	int index;
	float intensity;
	bool shadow;
	Scene* scene;
};

class DirLight : public Light
{
public:
	DirLight(glm::vec3 _dir, float _intensity, bool _shadow)
		:Light(_intensity, _shadow), dir(_dir) {};
	void setDir(glm::vec3 _dir);
	void setIntensity(float _intensity);
	glm::vec3 dir;
};

class PointLight : public Light
{
public:
	PointLight(glm::vec4 _pos, float _intensity, bool _shadow)
		: Light(_intensity, _shadow), pos(_pos) {};
	void setPos(glm::vec4 _pos);
	void setIntensity(float _intensity);
	glm::vec4 pos;
};

#endif
