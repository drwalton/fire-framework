#include "Renderable.hpp"

Renderable::Renderable(bool _translucent)
	:scene(nullptr),
	modelToWorld(glm::mat4(1.0)),
	translucent(_translucent)
{}

void Renderable::uniformScale(float s)
{
	modelToWorld = modelToWorld * glm::mat4(
		s  , 0.0, 0.0, 0.0,
		0.0, s  , 0.0, 0.0,
		0.0, 0.0, s  , 0.0,
		0.0, 0.0, 0.0, 1.0);
}

glm::vec4 Renderable::getOrigin()
{
	glm::vec4 o = glm::vec4(0.0, 0.0, 0.0, 1.0);
	o.x = modelToWorld[3][0];
	o.y = modelToWorld[3][1];
	o.z = modelToWorld[3][2];
	return o;
}

Solid::Solid(Shader* _shader)
	:Renderable(false), shader(_shader)
{
	material.ambient = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	material.diffuse = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	material.specular = glm::vec4(0.5f, 0.5f, 0.5f, 0.5f);
	material.exponent = 10.0f;
}

Solid::Solid(Shader* _shader, const Material& _material)
	:Renderable(false), shader(_shader),
	 material(_material)
{}

void Solid::setMaterial(const Material& _material)
{
	material = _material;
}
