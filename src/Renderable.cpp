#include "Renderable.hpp"

#include "Shader.hpp"

#include <gtc/matrix_transform.hpp>

Renderable::Renderable(bool _translucent)
	:scene(nullptr),
	modelToWorld(glm::mat4(1.0)),
	translation(glm::mat4(1.0)),
	rotation(glm::mat4(1.0)),
	scaling(glm::mat4(1.0)),
	translucent(_translucent)
{}

void Renderable::translate(const glm::vec3& t)
{
	translation[3][0] += t.x;
	translation[3][1] += t.y;
	translation[3][2] += t.z;
	modelToWorld = translation * rotation * scaling;
}

void Renderable::moveTo(const glm::vec3& p)
{
	translation[3][0] = p.x;
	translation[3][1] = p.y;
	translation[3][2] = p.z;
	modelToWorld = translation * rotation * scaling;
}

void Renderable::uniformScale(float s)
{
	scaling = scaling * glm::mat4(
		s  , 0.0, 0.0, 0.0,
		0.0, s  , 0.0, 0.0,
		0.0, 0.0, s  , 0.0,
		0.0, 0.0, 0.0, 1.0);
	modelToWorld = translation * rotation * scaling;
}

void Renderable::rotate(float angle, const glm::vec3& axis)
{
	rotation = rotation * glm::rotate(glm::mat4(1.0f), angle, axis);
	modelToWorld = translation * rotation * scaling;
}

void Renderable::setRotation(const glm::mat4& rotation)
{
	this->rotation = rotation;
	modelToWorld = translation * rotation * scaling;
}

glm::vec4 Renderable::getOrigin()
{
	glm::vec4 o = glm::vec4(0.0, 0.0, 0.0, 1.0);
	o.x = translation[3][0];
	o.y = translation[3][1];
	o.z = translation[3][2];
	return o;
}

