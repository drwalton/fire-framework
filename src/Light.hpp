#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "SH.hpp"
#include "Scene.hpp"

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
	glm::vec3 getDir() {return dir;};
	void setIntensity(float _intensity);
private:
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
	glm::vec4 getPos() {return pos;};
	void setIntensity(float _intensity);
private:
	glm::vec4 pos;
};

/* SHLight
 * A SH projected lighting environment.
 */
class SHLight : public Light
{
public:
	template <typename Fn>
	SHLight(Fn func, float _intensity);
	template <typename Fn>
	void setFunc(Fn func);
	void setCoeffts(std::vector<float>);
	std::vector<float> getCoeffts() {return coeffts;};
	void setIntensity(float _intensity);
private:
	std::vector<float> coeffts;
};

template <typename Fn>
SHLight::SHLight(Fn func, float _intensity)
	:Light(_intensity)
{
	coeffts = SH::shProject(Scene::sqrtSHSamples, Scene::nSHBands, func);
};


#endif
