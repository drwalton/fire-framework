#include "Scene.hpp"

Scene::Scene()
	 :ambLight(0.2f), nPhongLights(0)
{
	camera = new Camera();
	int i;
	for(i = 0; i < maxPhongLights; ++i)
	{
		phongLights[i] = nullptr;
		lightPos[i] = glm::vec4(0.0f);
		lightDiffuse[i] = glm::vec4(0.0f);
		lightSpecular[i] = glm::vec4(0.0f);
		lightAttenuation[i] = 0.0f;
	}
}

Scene::~Scene()
{
	for(std::set<Renderable*>::iterator i = opaque.begin(); i != opaque.end(); ++i)
	{
		delete (*i);
	}

	for(std::set<Renderable*>::iterator i = translucent.begin(); i != translucent.end(); ++i)
	{
		delete (*i);
	}

	for(std::set<Shader*>::iterator i = shaders.begin();
		i != shaders.end(); ++i)
	{
		delete (*i);
	}

	delete camera;
	camera = nullptr;
}

void Scene::render()
{
	updateCamera();
	//Render opaque renderables first.
	for(std::set<Renderable*>::iterator i = opaque.begin(); i != opaque.end(); ++i)
	{
		(*i)->render();
	}
	//Render translucent ones second.
	for(std::set<Renderable*>::iterator i = translucent.begin(); i != translucent.end(); ++i)
	{
		(*i)->render();
	}
}

void Scene::update(int dTime)
{
	for(std::set<Renderable*>::iterator i = opaque.begin(); i != opaque.end(); ++i)
	{
		(*i)->update(dTime);
	}
	for(std::set<Renderable*>::iterator i = translucent.begin(); i != translucent.end(); ++i)
	{
		(*i)->update(dTime);
	}
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
	
	setAmbLight(ambLight);
	updatePhongLights();
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
	/* Check light to be added is valid, not already in scene */
	if(nPhongLights >= maxPhongLights || l == nullptr 
		|| l->scene != nullptr || l->index != -1) return nullptr;
	phongLights[nPhongLights]      = l;
	/* Add light's data to uniform buffers */
	lightPos[nPhongLights]         = l->getPos();
	lightDiffuse[nPhongLights]     = l->getDiffuse();
	lightSpecular[nPhongLights]    = l->getSpecular();
	lightAttenuation[nPhongLights] = l->getAttenuation();
	l->index = nPhongLights;
	l->scene = this;
	++nPhongLights;
	updatePhongLights();
	return l;
}

PhongLight* Scene::updateLight(PhongLight* l)
{
	/* Check light is actually in scene before updating */
	if(l->scene != this || l == nullptr) return nullptr;
	/* Check light's index is valid */
	if(l != phongLights[l->index] ||
		l->index < 0 || l->index >= nPhongLights)
		return nullptr; //TODO: throw exception?
	/* Update values in stored buffers */
	lightPos[l->index]         = l->getPos();
	lightDiffuse[l->index]     = l->getDiffuse();
	lightSpecular[l->index]    = l->getSpecular();
	lightAttenuation[l->index] = l->getAttenuation();
	updatePhongLights();
	return l;
}

PhongLight* Scene::remove(PhongLight* l)
{
	/* Check light is in scene first */
	if(l->scene != this || l == nullptr) return nullptr;
	/* Check light's index is valid */
	if(l != phongLights[l->index] ||
		l->index < 0 || l->index >= nPhongLights)
		return nullptr; //TODO: throw exception ?
	/* Shift lights to fill gap left by removed light */
	for(int i = l->index; i < nPhongLights-1; ++i)
	{
		phongLights[i] = phongLights[i+1];
		lightPos[i] = lightPos[i+1];
		lightDiffuse[i] = lightDiffuse[i+1];
		lightSpecular[i] = lightSpecular[i+1];
		lightAttenuation[i] = lightAttenuation[i+1];
	}
	phongLights[nPhongLights-1] = nullptr;
	lightPos[nPhongLights-1] = glm::vec4(0.0f);
	lightDiffuse[nPhongLights-1] = glm::vec4(0.0f);
	lightSpecular[nPhongLights-1] = glm::vec4(0.0f);
	lightAttenuation[nPhongLights-1] = 0.0f;
	--nPhongLights;
	l->index = -1;
	l->scene = nullptr;
	updatePhongLights();
	return l;
}

void Scene::setAmbLight(glm::vec4 _ambLight)
{
	ambLight = _ambLight;
	for(std::set<Shader*>::iterator i = shaders.begin();
		i != shaders.end(); ++i)
	{
		(*i)->setAmbLight(ambLight);
	}
}

void Scene::updatePhongLights()
{
	for(std::set<Shader*>::iterator i = shaders.begin();
		i != shaders.end(); ++i)
	{
		(*i)->setPhongLights(lightPos, lightDiffuse,
			lightSpecular, lightAttenuation);
	}
}

void Scene::updateCamera()
{
	for(std::set<Shader*>::iterator i = shaders.begin();
		i != shaders.end(); ++i)
	{
		(*i)->setWorldToCamera(camera->getMat());
	}
}
