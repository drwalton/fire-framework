#include "Light.hpp"

void PhongLight::setPos(glm::vec4 _pos)
{
	pos = _pos;
	if(scene) scene->updateLight(this);
}

void PhongLight::setDiffuse(glm::vec4 _diffuse)
{
	diffuse = _diffuse;
	if(scene) scene->updateLight(this);
}

void PhongLight::setSpecular(glm::vec4 _specular)
{
	specular = _specular;
	if(scene) scene->updateLight(this);
}

void PhongLight::setAttenuation(float _attenuation)
{
	attenuation = _attenuation;
	if(scene) scene->updateLight(this);
}
