#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "Element.hpp"
#include "Shader.hpp"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <exception>

class Scene;

class BadMaterialIndex : public std::exception {};

/* Renderable
 * A Renderable is an ADT for an Element which has a render() function called by it's owning Scene each frame.
 * All Renderable implementations must also provide an update() function, which may well be a do-nothing for static objects.
 */
class Renderable : public Element
{
public:
	Renderable(bool _translucent);
	const bool translucent;
	void setModelToWorld(const glm::mat4& newPos) {modelToWorld = newPos;};
	glm::mat4 getModelToWorld() {return modelToWorld;};
	void prependTransform(const glm::mat4& t) {modelToWorld = t * modelToWorld;};
	void appendTransform(const glm::mat4& t) {modelToWorld = modelToWorld * t;};
	void translate(const glm::vec3& t);
	void moveTo(const glm::vec3& p);
	void uniformScale(float s);
	glm::vec4 getOrigin(); //Return pos'n of model space origin in world space.
	virtual void update(int dTime) = 0;
	virtual void render() = 0;
	virtual Shader* getShader() = 0;
	Scene* scene; //Points to scene containing renderable (nullptr if not in scene).
	virtual void onAdd() {}; //Called when the renderable is added to the scene.
	virtual void onRemove() {}; //Called when the renderable is removed from the scene.
protected:
	glm::mat4 modelToWorld;
};

/* Solid
 * A Solid is an ADT for a Renderable consisting of a solid opaque mesh.
 * This class also contains functions returning simple geometric Solid objects.
 */
class Solid : public Renderable
{
public:
	Solid(Shader* _shader);
	Solid(Shader* _shader, const std::vector<Material>& _materials);
	Shader* getShader() {return (Shader*) shader;};
	void setMaterials(const std::vector<Material>& _materials);
	void setMaterial(unsigned index, const Material& _material);
	Material getMaterial(unsigned index);
	void setAmbient(unsigned index, const glm::vec4& _ambient);
	void setDiffuse(unsigned index, const glm::vec4& _diffuse);
	void setSpecular(unsigned index, const glm::vec4& _specular);
	void setExponent(unsigned index, float _exponent);
protected:
	Shader* shader;
	std::vector<Material> materials;
};

#endif
