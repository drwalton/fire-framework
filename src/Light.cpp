#include "Light.hpp"

void PhongLight::setPos(glm::vec4 _pos)
{
	pos = _pos;
	if(manager) manager->update(this);
}

void PhongLight::setDiffuse(glm::vec4 _diffuse)
{
	diffuse = _diffuse;
	if(manager) manager->update(this);
}

void PhongLight::setSpecular(glm::vec4 _specular)
{
	specular = _specular;
	if(manager) manager->update(this);
}

void PhongLight::setAttenuation(float _attenuation)
{
	attenuation = _attenuation;
	if(manager) manager->update(this);
}
