#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "SH.hpp"
#include "LightManager.hpp"
#include "Element.hpp"
#include "GC.hpp"

#include <glm.hpp>
#include <glew.h>

class PhongLight;
class SHLight;
class Element;
class Scene;
class PhongLightManager;
class SHLightManager;

/* Light
 * An Element which is a light source (ADT).
 * **Note** Ambient lights are not Light objects. Ambient light may only be 
 *			adjusted per-scene by calling Scene::setAmbLight().
 */
class Light : public Element
{
	
};

/* PhongLight
 * A light designed for simple Phong-style local lighting shaders.
 * Lights with a pos.w of 0.0 are directional lights (with direction pos.xyz).
 * Otherwise the light is assumed to be a point source (set pos.w = 1.0).
 */
class PhongLight : public Light
{
public:
	PhongLight(glm::vec4 _pos)
		:pos(_pos), diffuse(glm::vec4(0.001, 0.001, 0.001, 1.0)),
		 specular(glm::vec4(0.001, 0.001, 0.001, 1.0)), attenuation(3.0f),
		 index(-1), manager(nullptr) {};
	PhongLight(glm::vec4 _pos, glm::vec4 _diffuse,
		glm::vec4 _specular, float _attenuation)
		:pos(_pos), diffuse(_diffuse),
		 specular(_specular), attenuation(_attenuation),
		 index(-1), manager(nullptr) {};
	void setPos(glm::vec4 _pos);
	void setDiffuse(glm::vec4 _diffuse);
	void setSpecular(glm::vec4 _specular);
	void setAttenuation(float _attenuation);
	glm::vec4 getPos() {return pos;};
	glm::vec4 getDiffuse() {return diffuse;};
	glm::vec4 getSpecular() {return specular;};
	float getAttenuation() {return attenuation;};
	int index;
	PhongLightManager* manager;
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
	SHLight(Fn func);
	template <typename Fn>
	void setFunc(Fn func);
	void setCoeffts(std::vector<glm::vec4>);
	std::vector<glm::vec4> getCoeffts() {return coeffts;};
	int index;
	SHLightManager* manager;
private:
	std::vector<glm::vec4> coeffts;
};

template <typename Fn>
SHLight::SHLight(Fn func)
	:index(-1), manager(nullptr)
{
	coeffts = SH::shProject(GC::sqrtSHSamples, GC::nSHBands, func);
};


#endif
