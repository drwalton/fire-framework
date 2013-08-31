#include "Light.hpp"

#include "LightManager.hpp"
#include "Element.hpp"
#include "GC.hpp"
#include "SHMat.hpp"

#include <gtc/matrix_transform.hpp>

std::vector<glm::vec3> operator * (const std::vector<glm::vec3> vec, const float& f)
{
	std::vector<glm::vec3> ans = vec;
	for(auto i = ans.begin(); i != ans.end(); ++i)
		*i *= f;
	return ans;
}

std::vector<glm::vec3> operator * (const std::vector<glm::vec3> vec, const glm::vec3& f)
{
	std::vector<glm::vec3> ans = vec;
	for(auto i = ans.begin(); i != ans.end(); ++i)
		*i *= f;
	return ans;
}

void PhongLight::setPos(glm::vec4 _pos)
{
	pos = _pos;
	update();
}

void PhongLight::setDiffuse(glm::vec4 _diffuse)
{
	diffuse = _diffuse;
	update();
}

void PhongLight::setSpecular(glm::vec4 _specular)
{
	specular = _specular;
	update();
}

void PhongLight::setAttenuation(float _attenuation)
{
	attenuation = _attenuation;
	update();
}

void PhongLight::update()
{
	if(manager) manager->update(this);
}

void SHLight::setCoeffts(std::vector<glm::vec3> coeffts)
{
	this->coeffts = coeffts;
	retCoeffts = rotation * coeffts * intensity * color;
}

void SHLight::rotateCoeffts(const glm::mat4& rotation)
{
	this->rotation = SHMat(rotation, GC::nSHBands);
	retCoeffts = this->rotation * coeffts * intensity * color;
}

void SHLight::rotateCoeffts(const SHMat& rotation)
{
	this->rotation = rotation;
	retCoeffts = this->rotation * coeffts * intensity * color;
}

void SHLight::pointAt(glm::vec3 dir)
{
	dir = glm::normalize(dir);
	float theta = -asin(dir.y);
	float phi = -atan2(dir.z, dir.x);

	glm::mat4 look = glm::rotate(glm::mat4(1.0f),
		(theta * 180.0f) / PI,
		glm::vec3(1.0f, 0.0f, 0.0f));
	look = glm::rotate(look,
		(phi * 180.0f) / PI, 
		glm::vec3(0.0f, 1.0f, 0.0f));

	rotation = SHMat(
		look,
		GC::nSHBands);
	retCoeffts = rotation * coeffts * intensity * color;
}

void SHLight::setIntensity(float intensity)
{
	this->intensity = intensity;
	retCoeffts = rotation * coeffts * intensity * color;
}

void SHLight::setColor(const glm::vec3& color)
{
	this->color = color;
	retCoeffts = rotation * coeffts * intensity * color;
}
