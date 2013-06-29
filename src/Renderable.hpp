#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "Element.hpp"
#include "Shader.hpp"

#include <glm.hpp>

class Scene;

class Renderable;
class Solid;
template <int nVertices> class ArrSolid;
class ElemSolid;

class Renderable : public Element
{
public:
	Renderable(Shader* _render);
	void setModelToWorld(const glm::mat4& newPos) {modelToWorld = newPos;};
	void concatTransform(const glm::mat4& t) {modelToWorld = t * modelToWorld;};
	virtual void update(int dTime) = 0;
	virtual void render() = 0;
	Shader* renderShader;
	Scene* scene; //Points to scene containing renderable (nullptr if not in scene).
protected:
	glm::mat4 modelToWorld;
};

class Solid : public Renderable
{
public:
	Solid(LightShader* _render) :Renderable(_render) {};
	static ArrSolid<36>* Cube(LightShader* _render);
};

/* ArrSolid
 * Solid using array style drawing. 
 */
template<int nVertices>
class ArrSolid : public Solid
{
public:
	ArrSolid(LightShader* _render, 
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
class ElemSolid : public Solid
{
public:
private:
	int numVertices;
	int numElements;
	glm::mat4* v;
	glm::vec3* n;
	GLushort* e;
	GLuint v_attrib;
	GLuint n_attrib;
	GLuint e_attrib;
};


#endif
