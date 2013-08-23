#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "Element.hpp"

#include <glm.hpp>
#include <exception>

class Scene;
class Shader;

class BadMaterialIndex : public std::exception {};

/* Renderable
 * A Renderable is an ADT for an Element which has a render()
 *   function called by it's owning Scene each frame.
 * All Renderable implementations must also provide an update()
 *   function, which may well be a no-op for static objects.
 * Renderables have a boolean member called `translucent'. If 
 *   false, the Renderable is considered entirely opaque. If
 *   true, some degree of translucency is assumed and the 
 *   alpha blending will be used.
 */
class Renderable : public Element
{
public:
	Renderable(bool _translucent);
	const bool translucent;
	glm::mat4 getModelToWorld() {return modelToWorld;};
	glm::mat4 getRotation() {return rotation;};
	glm::mat4 getTranslation() {return translation;};
	glm::mat4 getScaling() {return scaling;};
	void translate(const glm::vec3& t);
	void moveTo(const glm::vec3& p);
	void uniformScale(float s);
	void rotate(float angle, const glm::vec3& axis);
	void setRotation(const glm::mat4& rotation);
	glm::vec4 getOrigin(); //Return pos'n of model space origin in world space.
	virtual void update(int dTime) = 0;
	virtual void render() = 0;
	virtual Shader* getShader() = 0;
	Scene* scene; //Points to scene containing renderable (nullptr if not in scene).
	virtual void onAdd() {}; //Called when the renderable is added to the scene.
	virtual void onRemove() {}; //Called when the renderable is removed from the scene.
protected:
	glm::mat4 translation;
	glm::mat4 rotation;
	glm::mat4 scaling;
	glm::mat4 modelToWorld;
};

#endif
