#include "LightManager.hpp"

#include <algorithm>

PhongLightManager::PhongLightManager()
{
	for(int i = 0; i < GC::maxPhongLights; ++i)
	{
		lights[i] = nullptr;
		block.lightPos[i] = glm::vec4(0.0f);
		block.lightDiffuse[i] = glm::vec4(0.0f);
		block.lightSpecular[i] = glm::vec4(0.0f);
		block.lightAttenuation[i] = 0.0f;
	}
	nLights = 0;

	glGenBuffers(1, &block_ubo);
	glBindBufferRange(GL_UNIFORM_BUFFER, Shader::getUBlockBindingIndex("phongBlock"),
		block_ubo, 0, sizeof(block));
	glBufferData(GL_UNIFORM_BUFFER, sizeof(block), &(block), GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

PhongLight* PhongLightManager::add(PhongLight* l)
{
	/* Check light to be added is valid, not already in scene */
	if(nLights >= GC::maxPhongLights || l->manager != nullptr
		|| l == nullptr || l->index != -1) return nullptr;
	lights[nLights] = l;
	/* Add light's data to uniform buffers */
	block.lightPos[nLights]         = l->getPos();
	block.lightDiffuse[nLights]     = l->getDiffuse();
	block.lightSpecular[nLights]    = l->getSpecular();
	block.lightAttenuation[nLights] = l->getAttenuation();
	l->index = nLights;
	l->manager = this;
	++nLights;
	updateBlock();
	return l;
}

PhongLight* PhongLightManager::update(PhongLight* l)
{
	/* Check light is actually in manager before updating */
	if(l->manager != this || l == nullptr) return nullptr;
	/* Check light's index is valid */
	if(l->index < 0 || l->index >= nLights) return nullptr;
	if(l != lights[l->index]) return nullptr;
	/* Update values in stored buffer */
	block.lightPos[l->index]         = l->getPos();
	block.lightDiffuse[l->index]     = l->getDiffuse();
	block.lightSpecular[l->index]    = l->getSpecular();
	block.lightAttenuation[l->index] = l->getAttenuation();
	++nLights;
	updateBlock();
	return l;
}

PhongLight* PhongLightManager::remove(PhongLight* l)
{
	/* Check light is in manager first */
	if(l->manager != this || l == nullptr) return nullptr;
	/* Check light's index is valid */
	if(l != lights[l->index] ||
		l->index < 0 || l->index >= nLights)
		return nullptr; //TODO: throw exception ?
	/* Shift lights to fill gap left by removed light */
	for(int i = l->index; i < nLights-1; ++i)
		lights[i] = lights[i+1];
	lights[nLights-1] = nullptr;
	--nLights;
	l->index = -1;
	l->manager = nullptr;
	return l;
}

void PhongLightManager::updateBlock()
{
	glBindBuffer(GL_UNIFORM_BUFFER, block_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(block), &(block));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

SHLightManager::SHLightManager()
{
	glGenBuffers(1, &block_ubo);
	glBindBufferRange(GL_UNIFORM_BUFFER, Shader::getUBlockBindingIndex("SHBlock"),
	block_ubo, 0, sizeof(block));
	glBufferData(GL_UNIFORM_BUFFER, sizeof(block), &(block), GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


SHLight* SHLightManager::add(SHLight* l)
{
	/* Check light to be added is valid, not already in some manager */
	if(l == nullptr || l->manager != nullptr) return nullptr;
	lights.insert(l);
	l->manager = this;
	return l;
}

SHLight* SHLightManager::remove(SHLight* l)
{
	lights.erase(l);
	l->manager = nullptr;
	return l;
}

void SHLightManager::update()
{
	std::fill(block.lightCoeffts, block.lightCoeffts + GC::nSHCoeffts,
		glm::vec4(0.0f));

	for(auto l = lights.begin(); l != lights.end(); ++l)
		for(int c = 0; c < GC::nSHCoeffts; ++c)
		{
			glm::vec3 lc = (*l)->getCoeffts()[c];
			block.lightCoeffts[c] += glm::vec4(lc.x, lc.y, lc.z, 0.0f);
		}

	glBindBuffer(GL_UNIFORM_BUFFER, block_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(block), &(block));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
