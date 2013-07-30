#include "Light.hpp"

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

void SHLight::setCoeffts(std::vector<glm::vec4> _coeffts)
{
	coeffts = _coeffts;
	update();
}

void SHLight::rotateCoeffts(glm::mat4 _rotation)
{
	rotation = SHMat(_rotation, GC::nSHBands);
	update();
}

void SHLight::update()
{
	if(manager) manager->update(this);
}
