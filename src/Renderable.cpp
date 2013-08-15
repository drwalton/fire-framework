#include "Renderable.hpp"

#include "Shader.hpp"

#include <gtc/matrix_transform.hpp>

Renderable::Renderable(bool _translucent)
	:scene(nullptr),
	modelToWorld(glm::mat4(1.0)),
	translucent(_translucent)
{}

void Renderable::translate(const glm::vec3& t)
{
	modelToWorld[3][0] += t.x;
	modelToWorld[3][1] += t.y;
	modelToWorld[3][2] += t.z;
}

void Renderable::moveTo(const glm::vec3& p)
{
	modelToWorld[3][0] = p.x;
	modelToWorld[3][1] = p.y;
	modelToWorld[3][2] = p.z;
}

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

