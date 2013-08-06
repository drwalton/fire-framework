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

void SHLight::setCoeffts(std::vector<glm::vec3> _coeffts)
{
	coeffts = _coeffts;
	rotCoeffts = rotation * coeffts;
}

void SHLight::rotateCoeffts(glm::mat4 _rotation)
{
	rotation = SHMat(_rotation, GC::nSHBands);
	rotCoeffts = rotation * coeffts;
}
