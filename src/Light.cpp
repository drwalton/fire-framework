#include "Light.hpp"
#include "Scene.hpp"

void DirLight::setDir(glm::vec3 _dir)
{
	dir = _dir;
	if(!scene) return;
	scene->updateLight(this);
}

void DirLight::setIntensity(float _intensity)
{
	intensity = _intensity;
	if(!scene) return;
	scene->updateLight(this);
}

void PointLight::setPos(glm::vec4 _pos)
{
	pos = _pos;
	if(!scene) return;
	scene->updateLight(this);
}

void PointLight::setIntensity(float _intensity)
{
	intensity = _intensity;
	if(!scene) return;
	scene->updateLight(this);
}
