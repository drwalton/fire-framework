#include "Renderable.hpp"

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

Solid::Solid(Shader* _shader)
	:Renderable(false), shader(_shader)
{
	Material material;
	material.ambient = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	material.diffuse = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	material.specular = glm::vec4(0.5f, 0.5f, 0.5f, 0.5f);
	material.exponent = 10.0f;
	materials.push_back(material);
}

Solid::Solid(Shader* _shader, const std::vector<Material>& _materials)
	:Renderable(false), shader(_shader),
	 materials(_materials)
{}

void Solid::setMaterials(const std::vector<Material>& _materials)
{
	materials = _materials;
}

void Solid::setMaterial(unsigned index, const Material& _material)
{
	if(index < 0 || index >= materials.size()) throw(new BadMaterialIndex);
	materials[index] = _material;
}

Material Solid::getMaterial(unsigned index)
{
	if(index < 0 || index >= materials.size()) throw(new BadMaterialIndex);
	return materials[index];
}

void Solid::setAmbient(unsigned index, const glm::vec4& _ambient)
{
	if(index < 0 || index >= materials.size()) throw(new BadMaterialIndex);
	materials[index].ambient = _ambient;
}

void Solid::setDiffuse(unsigned index, const glm::vec4& _diffuse)
{
	if(index < 0 || index >= materials.size()) throw(new BadMaterialIndex);
	materials[index].diffuse = _diffuse;
}

void Solid::setSpecular(unsigned index, const glm::vec4& _specular)
{
	if(index < 0 || index >= materials.size()) throw(new BadMaterialIndex);
	materials[index].specular = _specular;
}

void Solid::setExponent(unsigned index, float _exponent)
{
	if(index < 0 || index >= materials.size()) throw(new BadMaterialIndex);
	materials[index].exponent = _exponent;
}
