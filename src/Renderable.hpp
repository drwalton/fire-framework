#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "Element.hpp"
#include "Shader.hpp"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

class Scene;

class Renderable;
class Solid;
template <int nVertices> class ArrSolid;
template <int nVertices, int nElements> class ElemSolid;

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
	void prependTransform(const glm::mat4& t) {modelToWorld = t * modelToWorld;};
	void appendTransform(const glm::mat4& t) {modelToWorld = modelToWorld * t;};
	void translate(const glm::vec3& t) {modelToWorld = glm::translate(modelToWorld, t);};
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
 * A Solid is an ADT for a Renderable consisting of a solid mesh.
 * This class also contains functions returning simple geometric Solid objects.
 */
class Solid : public Renderable
{
public:
	Solid(LightShader* _shader) :Renderable(false), shader(_shader) {};
	Shader* getShader() {return (Shader*) shader;};

	static ArrSolid<36>* Cube(LightShader* _shader);
	static ArrSolid<6>*  Quad(LightShader* _shader);
protected:
	LightShader* shader;
};

/* ArrSolid
 * Solid using array style drawing. 
 */
template<int nVertices>
class ArrSolid : public Solid
{
public:
	ArrSolid(LightShader* _shader, 
		const std::array<glm::vec4, nVertices>& _v, 
		const std::array<glm::vec3, nVertices>& _n);	
	void render();
	void update(int dTime) {};
private:
	std::array<glm::vec4, nVertices> v; //Vertices
	std::array<glm::vec3, nVertices> n; //Vertex norms
	GLuint v_vbo;
	GLuint n_vbo;
	GLuint v_attrib;
	GLuint n_attrib;
};

/* ElemSolid
 * Solid using indexed drawing.
 */
template<int nVertices, int nElements>
class ElemSolid : public Solid
{
public:
private:
	std::array<glm::vec4, nVertices> v;
	std::array<glm::vec3, nVertices> n;
	std::array<GLushort, nElements> e;
	GLuint v_vbo;
	GLuint n_vbo;
	GLuint v_attrib;
	GLuint n_attrib;
	GLuint e_attrib;
};

#endif
