#include "Scene.hpp"

Scene::Scene()
	 :ambLight(0.2f), nDirLights(0), 
		nPointLights(0) 
{
	camera = new Camera();
	int i;
	for(i = 0; i < maxDirLights; ++i)
	{
		dirLights[i] = nullptr;
		dirLightOn[i] = 0;
		dirLightDir[i] = glm::vec3(0.0f);
		dirIntensity[i] = -1.0f;
	}
	for(i = 0; i < maxPointLights; ++i)
	{
		pointLights[i] = nullptr;
		pointLightOn[i] = 0;
		pointLightPos[i] = glm::vec4(0.0f);
		pointIntensity[i] = -1.0f;
	}
}

Scene::~Scene()
{
	for(std::set<Renderable*>::iterator i = renderables.begin(); i != renderables.end(); ++i)
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
	for(std::set<Renderable*>::iterator i = renderables.begin(); i != renderables.end(); ++i)
	{
		(*i)->render();
	}
}

void Scene::update(int dTime)
{
	for(std::set<Renderable*>::iterator i = renderables.begin(); i != renderables.end(); ++i)
	{
		(*i)->update(dTime);
	}
}

Renderable* Scene::add(Renderable* const r)
{
	if(r == nullptr) return;
	renderables.insert(r);
	r->scene = this;
	shaders.insert(r->renderShader);

	
	setAmbLight(ambLight);
	updateDirLights();
	updatePointLights();
	return r;
}

Renderable* Scene::remove(Renderable* r)
{
	renderables.erase(r);
	//TODO Remove shaders if appropriate (not needed by another renderable).
	return r;
}

DirLight* Scene::add(DirLight* d)
{
	if(nDirLights >= maxDirLights) return nullptr;
	dirLights[nDirLights] = d;
	dirLightOn[nDirLights] = d->on ? 1 : 0;
	dirLightDir[nDirLights] = d->dir;
	dirIntensity[nDirLights] = d->intensity;
	d->index = nDirLights;
	d->scene = this;
	++nDirLights;
	updateDirLights();
	return d;
}

DirLight* Scene::updateLight(DirLight* d)
{
	if(d->scene != this || d->index == -1 || dirLights[d->index] != d) 
		return nullptr;
	dirLightOn[d->index] = d->on ? 1 : 0;
	dirLightDir[d->index] = d->dir;
	dirIntensity[d->index] = d->intensity;
	updateDirLights(); 
	return d;
}

DirLight* Scene::remove(DirLight* d)
{
	if(nDirLights <= 0 || d->index == -1) return nullptr;
	for(int i = d->index; i < nDirLights-1; ++i)
	{
		dirLights[i] = dirLights[i+1];
		dirLightOn[i] = dirLightOn[i+1];
		dirLights[i]->index = i;
		dirLightDir[i] = dirLightDir[i+1];
		dirIntensity[i] = dirIntensity[i+1];
	}
	dirLights[nDirLights-1] = nullptr;
	dirLightOn[nDirLights-1] = 0;
	dirLightDir[nDirLights-1] = glm::vec3(0.0);
	dirIntensity[nDirLights-1] = -1.0;
	--nDirLights;
	d->index = -1;
	updateDirLights();
	return d;
}

PointLight* Scene::add(PointLight* p)
{
	if(nPointLights >= maxPointLights) return nullptr;
	pointLightOn[nPointLights] = p->on ? 1 : 0;
	pointLights[nPointLights] = p;
	pointLightPos[nPointLights] = p->pos;
	pointIntensity[nPointLights] = p->intensity;
	p->index = nPointLights;
	p->scene = this;
	++nPointLights;
	updatePointLights();
	return p;
}

PointLight* Scene::updateLight(PointLight* p)
{
	if(p->scene != this || p->index == -1) return nullptr;
	pointLightOn[p->index] = p->on ? 1 : 0;
	pointLightPos[p->index] = p->pos;
	pointIntensity[p->index] = p->intensity;
	updatePointLights();
	return p;
}

PointLight* Scene::remove(PointLight* p)
{
	if(nPointLights <= 0 || p->index == -1) return nullptr;
	for(int i = p->index; i < nPointLights-1; ++i)
	{
		pointLights[i] = pointLights[i+1];
		pointLights[i]->index = i;
		pointLightOn[i] = pointLightOn[i+1];
		pointLightPos[i] = pointLightPos[i+1];
		pointIntensity[i] = pointIntensity[i+1];
	}
	pointLights[nPointLights-1] = nullptr;
	pointLightOn[nPointLights-1] = 0;
	pointLightPos[nPointLights-1] = glm::vec4(0.0, 0.0, 0.0, 1.0);
	pointIntensity[nPointLights-1] = -1.0;
	--nDirLights;
	p->index = -1;
	updatePointLights();
	return p;
}

AdvectParticlesRandLights* Scene::add(AdvectParticlesRandLights* a)
{
	// Check there is room for the lights.
	if(maxPointLights - nPointLights < a->nLights) return nullptr;
	// Add lights
	for(int i = 0; i < a->nLights; ++i)
	{
		add(a->lights[i]);
	}
	add((Renderable*) a);
	return a;
}

AdvectParticlesRandLights* Scene::remove(AdvectParticlesRandLights* a)
{
	for (int i = 0; i < a->nLights; ++i)
	{
		remove(a->lights[i]);
	}
	remove((Renderable*) a);
	return a;
}

//TODO replace with checking by type of shader.
void Scene::setAmbLight(float _ambLight)
{
	ambLight = _ambLight;
	for(std::set<Shader*>::iterator i = shaders.begin();
		i != shaders.end(); ++i)
	{
		if((*i)->hasAmbLight)
			((LightShader*)(*i))->setAmbLight(ambLight);
	}
}

void Scene::updateDirLights()
{
	for(std::set<Shader*>::iterator i = shaders.begin();
		i != shaders.end(); ++i)
	{
		if((*i)->hasDirLights)
			((LightShader*)(*i))->setDirLights(dirLightOn, 
				dirLightDir, dirIntensity, maxDirLights);
	}
}

void Scene::updatePointLights()
{
	for(std::set<Shader*>::iterator i = shaders.begin();
		i != shaders.end(); ++i)
	{
		if((*i)->hasPointLights)
			((LightShader*)(*i))->setPointLights(pointLightOn, pointLightPos, 
				pointIntensity, maxPointLights);
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
