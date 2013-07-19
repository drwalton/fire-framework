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
 *			adjusted per-scene by calling Scene::setAmbLight().
 */
class Light : public Element
{};

/* PhongLight
 * A light designed for simple Phong-style local lighting shaders.
 * Lights with a pos.w of 0.0 are directional lights (with direction pos.xyz).
 * Otherwise the light is assumed to be a point source (set pos.w = 1.0).
 */
class PhongLight : public Light
{
public:
	PhongLight(glm::vec4 _pos)
		:pos(_pos), diffuse(glm::vec4(1.0, 1.0, 1.0, 1.0)),
		 specular(glm::vec4(1.0, 1.0, 1.0, 1.0)), attenuation(1.0f)
		 scene(nullptr), index(-1) {};
	PhongLight(glm::vec4 _pos, glm::vec4 _diffuse,
		glm::vec4 _specular, float _attenuation)
		:pos(_pos), diffuse(_diffuse),
		 specular(_specular), attenuation(_attenuation),
		 scene(nullptr), index(-1) {};
	void setPos(glm::vec4 _pos);
	void setDiffuse(glm::vec4 _diffuse);
	void setSpecular(glm::vec4 _specular);
	void setAttenuation(float _attenuation);
	glm::vec4 getPos() {return pos;};
	glm::vec4 getDiffuse() {return diffuse;};
	glm::vec4 getSpecular() {return specular;};
	float getAttenuation() {return attenuation;};
	int index;
	Scene* scene;
private:
	glm::vec4 pos;
	glm::vec4 diffuse;
	glm::vec4 specular;
	float attenuation;
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
