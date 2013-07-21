#include "Scene.hpp"

Scene::Scene()
	 :ambLight(0.1f, 0.1f, 0.1f, 1.0f)
{
	camera = new Camera();
	int i;
	for(i = 0; i < maxPhongLights; ++i)
	{
		phongLights[i] = nullptr;
		phong.lightPos[i] = glm::vec4(0.0f);
		phong.lightDiffuse[i] = glm::vec4(0.0f);
		phong.lightSpecular[i] = glm::vec4(0.0f);
		phong.lightAttenuation[i] = 0.0f;
	}
	phong.nLights = 0;

	for(i = 0; i < maxSHLights; ++i)
		SHLights[i] = nullptr;
	for(i = 0; i < maxSHLights*nSHCoeffts; ++i)
		sh.lightCoeffts[i] = glm::vec4(0.0f);
	sh.nLights = 0;

	glGenBuffers(1, &ambBlock_ubo);
	glBindBufferRange(GL_UNIFORM_BUFFER, Shader::getUBlockBindingIndex("ambBlock"), ambBlock_ubo, 0, sizeof(ambLight));
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ambLight), &(ambLight[0]), GL_STREAM_DRAW);

	glGenBuffers(1, &phongBlock_ubo);
	glBindBufferRange(GL_UNIFORM_BUFFER, Shader::getUBlockBindingIndex("phongBlock"), phongBlock_ubo, 0, sizeof(phong));
	glBufferData(GL_UNIFORM_BUFFER, sizeof(phong), &(phong), GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glGenBuffers(1, &SHBlock_ubo);
	glBindBufferRange(GL_UNIFORM_BUFFER, Shader::getUBlockBindingIndex("SHBlock"), SHBlock_ubo, 0, sizeof(sh));
	glBufferData(GL_UNIFORM_BUFFER, sizeof(sh), &(sh), GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
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
	if(phong.nLights >= maxPhongLights || l == nullptr 
		|| l->scene != nullptr || l->index != -1) return nullptr;
	phongLights[phong.nLights]      = l;
	/* Add light's data to uniform buffers */
	phong.lightPos[phong.nLights]         = l->getPos();
	phong.lightDiffuse[phong.nLights]     = l->getDiffuse();
	phong.lightSpecular[phong.nLights]    = l->getSpecular();
	phong.lightAttenuation[phong.nLights] = l->getAttenuation();
	l->index = phong.nLights;
	l->scene = this;
	++phong.nLights;
	updatePhongLights();
	return l;
}

PhongLight* Scene::updateLight(PhongLight* l)
{
	/* Check light is actually in scene before updating */
	if(l->scene != this || l == nullptr) return nullptr;
	/* Check light's index is valid */
	if(l != phongLights[l->index] ||
		l->index < 0 || l->index >= phong.nLights)
		return nullptr; //TODO: throw exception?
	/* Update values in stored buffers */
	phong.lightPos[l->index]         = l->getPos();
	phong.lightDiffuse[l->index]     = l->getDiffuse();
	phong.lightSpecular[l->index]    = l->getSpecular();
	phong.lightAttenuation[l->index] = l->getAttenuation();
	updatePhongLights();
	return l;
}

PhongLight* Scene::remove(PhongLight* l)
{
	/* Check light is in scene first */
	if(l->scene != this || l == nullptr) return nullptr;
	/* Check light's index is valid */
	if(l != phongLights[l->index] ||
		l->index < 0 || l->index >= phong.nLights)
		return nullptr; //TODO: throw exception ?
	/* Shift lights to fill gap left by removed light */
	for(int i = l->index; i < phong.nLights-1; ++i)
	{
		phongLights[i] = phongLights[i+1];
		phong.lightPos[i] = phong.lightPos[i+1];
		phong.lightDiffuse[i] = phong.lightDiffuse[i+1];
		phong.lightSpecular[i] = phong.lightSpecular[i+1];
		phong.lightAttenuation[i] = phong.lightAttenuation[i+1];
	}
	phongLights[phong.nLights-1] = nullptr;
	phong.lightPos[phong.nLights-1] = glm::vec4(0.0f);
	phong.lightDiffuse[phong.nLights-1] = glm::vec4(0.0f);
	phong.lightSpecular[phong.nLights-1] = glm::vec4(0.0f);
	phong.lightAttenuation[phong.nLights-1] = 0.0f;
	--phong.nLights;
	l->index = -1;
	l->scene = nullptr;
	updatePhongLights();
	return l;
}

SHLight* Scene::add(SHLight* l)
{
	/* Check light to be added is valid, not already in scene */
	if(sh.nLights >= maxSHLights || l == nullptr 
		|| l->scene != nullptr || l->index != -1) return nullptr;
	SHLights[sh.nLights] = l;
	/* Add light's data to uniform buffers */
	for(int c = 0; c < nSHCoeffts; ++c)
		sh.lightCoeffts[sh.nLights*nSHCoeffts + c] = l->getCoeffts()[c];
	l->index = sh.nLights;
	l->scene = this;
	++sh.nLights;
	updateSHLights();
	return l;
}

SHLight* Scene::updateLight(SHLight* l)
{
	/* Check light is actually in scene before updating */
	if(l->scene != this || l == nullptr) return nullptr;
	/* Check light's index is valid */
	if(l != SHLights[l->index] ||
		l->index < 0 || l->index >= sh.nLights)
		return nullptr; //TODO: throw exception?
	/* Update values in stored buffers */
	for(int c = 0; c < nSHCoeffts; ++c)
		sh.lightCoeffts[sh.nLights*nSHCoeffts + c] = l->getCoeffts()[c];
	updateSHLights();
	return l;
}

SHLight* Scene::remove(SHLight* l)
{
	/* Check light is in scene first */
	if(l->scene != this || l == nullptr) return nullptr;
	/* Check light's index is valid */
	if(l != SHLights[l->index] ||
		l->index < 0 || l->index >= sh.nLights)
		return nullptr; //TODO: throw exception ?
	/* Shift lights to fill gap left by removed light */
	for(int i = l->index; i < sh.nLights-1; ++i)
	{
		SHLights[i] = SHLights[i+1];
		for(int c = 0; c < nSHCoeffts; ++c)
			sh.lightCoeffts[i*nSHCoeffts + c] = sh.lightCoeffts[(i+1)*nSHCoeffts + c];
	}
	SHLights[sh.nLights-1] = nullptr;
	for(int c = 0; c < nSHCoeffts; ++c)
		sh.lightCoeffts[(sh.nLights)*nSHCoeffts + c] = glm::vec4(0.0f);
	--sh.nLights;
	l->index = -1;
	l->scene = nullptr;
	updateSHLights();
	return l;
}

void Scene::setAmbLight(glm::vec4 _ambLight)
{
	ambLight = _ambLight;
	glBindBuffer(GL_UNIFORM_BUFFER, ambBlock_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ambLight), &(ambLight[0]));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Scene::updatePhongLights()
{
	glBindBuffer(GL_UNIFORM_BUFFER, phongBlock_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(phong), &(phong));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Scene::updateSHLights()
{
	glBindBuffer(GL_UNIFORM_BUFFER, SHBlock_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(sh), &(sh));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
