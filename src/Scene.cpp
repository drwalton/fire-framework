#include "Scene.hpp"
#include "Renderable.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "GC.hpp"
#include "SpherePlot.hpp"

#include <GL/glut.h>
#include <SOIL.h>

#include <vector>

Scene::Scene()
	 :ambLight(0.1f, 0.1f, 0.1f, 1.0f)
{
	camera = new Camera();

	glGenBuffers(1, &ambBlock_ubo);
	glBindBufferRange(GL_UNIFORM_BUFFER, Shader::getUBlockBindingIndex("ambBlock"),
		ambBlock_ubo, 0, sizeof(ambLight));
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ambLight), &(ambLight[0]), GL_STREAM_DRAW);
}

Scene::~Scene()
{
	for(auto i = opaque.begin(); i != opaque.end(); ++i)
	{
		delete (*i);
	}

	for(auto i = translucent.begin(); i != translucent.end(); ++i)
	{
		delete (*i);
	}

	for(auto i = shaders.begin(); i != shaders.end(); ++i)
	{
		delete (*i);
	}

	delete camera;
	camera = nullptr;
}

void Scene::render()
{
	//Render opaque renderables first.
	for(auto i = opaque.begin(); i != opaque.end(); ++i)
	{
		(*i)->render();
	}
	//Render translucent ones second.
	for(auto i = translucent.begin(); i != translucent.end(); ++i)
	{
		(*i)->render();
	}
}

void Scene::update(int dTime)
{
	for(auto i = opaque.begin(); i != opaque.end(); ++i)
	{
		(*i)->update(dTime);
	}
	for(auto i = translucent.begin(); i != translucent.end(); ++i)
	{
		(*i)->update(dTime);
	}

	shManager.update();
}

Renderable* Scene::add(Renderable* const r)
{
	if(r == nullptr) return nullptr;
	if(r->translucent)
		translucent.insert(r);
	else
		opaque.insert(r);
	r->scene = this;
	shaders.insert(r->getShader());

	r->onAdd();
	return r;
}

Renderable* Scene::remove(Renderable* r)
{
	if(r->translucent)
		translucent.erase(r);
	else
		opaque.erase(r);
	//TODO Remove shaders if appropriate (not needed by another renderable).
	r->onRemove();
	return r;
}

PhongLight* Scene::add(PhongLight* l)
{
	return phongManager.add(l);
}

PhongLight* Scene::remove(PhongLight* l)
{
	return phongManager.remove(l);
}

SHLight* Scene::add(SHLight* l)
{
	return shManager.add(l);
}

SHLight* Scene::remove(SHLight* l)
{
	return shManager.remove(l);
}

void Scene::setAmbLight(glm::vec4 _ambLight)
{
	ambLight = _ambLight;
	glBindBuffer(GL_UNIFORM_BUFFER, ambBlock_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ambLight), &(ambLight[0]));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
