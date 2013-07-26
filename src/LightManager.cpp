#include "LightManager.hpp"

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
	block.nLights = 0;

	glGenBuffers(1, &block_ubo);
	glBindBufferRange(GL_UNIFORM_BUFFER, Shader::getUBlockBindingIndex("phongBlock"), block_ubo, 0, sizeof(block));
	glBufferData(GL_UNIFORM_BUFFER, sizeof(block), &(block), GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

PhongLight* PhongLightManager::add(PhongLight* l)
{
	/* Check light to be added is valid, not already in scene */
	if(block.nLights >= GC::maxPhongLights || l->manager != nullptr
		|| l == nullptr || l->index != -1) return nullptr;
	lights[block.nLights] = l;
	/* Add light's data to uniform buffers */
	block.lightPos[block.nLights]         = l->getPos();
	block.lightDiffuse[block.nLights]     = l->getDiffuse();
	block.lightSpecular[block.nLights]    = l->getSpecular();
	block.lightAttenuation[block.nLights] = l->getAttenuation();
	l->index = block.nLights;
	l->manager = this;
	++block.nLights;
	updateBlock();
	return l;
}

PhongLight* PhongLightManager::update(PhongLight* l)
{
	/* Check light is actually in manager before updating */
	if(l->manager != this || l == nullptr) return nullptr;
	/* Check light's index is valid */
	if(l->index < 0 || l->index >= block.nLights) return nullptr;
	if(l != lights[l->index]) return nullptr;
	/* Update values in stored buffer */
	block.lightPos[l->index]         = l->getPos();
	block.lightDiffuse[l->index]     = l->getDiffuse();
	block.lightSpecular[l->index]    = l->getSpecular();
	block.lightAttenuation[l->index] = l->getAttenuation();
	updateBlock();
	return l;
}

PhongLight* PhongLightManager::remove(PhongLight* l)
{
	/* Check light is in manager first */
	if(l->manager != this || l == nullptr) return nullptr;
	/* Check light's index is valid */
	if(l != lights[l->index] ||
		l->index < 0 || l->index >= block.nLights)
		return nullptr; //TODO: throw exception ?
	/* Shift lights to fill gap left by removed light */
	for(int i = l->index; i < block.nLights-1; ++i)
	{
		lights[i] = lights[i+1];
		block.lightPos[i] = block.lightPos[i+1];
		block.lightDiffuse[i] = block.lightDiffuse[i+1];
		block.lightSpecular[i] = block.lightSpecular[i+1];
		block.lightAttenuation[i] = block.lightAttenuation[i+1];
	}
	lights[block.nLights-1] = nullptr;
	block.lightPos[block.nLights-1] = glm::vec4(0.0f);
	block.lightDiffuse[block.nLights-1] = glm::vec4(0.0f);
	block.lightSpecular[block.nLights-1] = glm::vec4(0.0f);
	block.lightAttenuation[block.nLights-1] = 0.0f;
	--block.nLights;
	l->index = -1;
	l->manager = nullptr;
	updateBlock();
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
	for(int i = 0; i < GC::maxSHLights; ++i)
		lights[i] = nullptr;
	for(int i = 0; i < GC::maxSHLights*GC::nSHCoeffts; ++i)
		block.lightCoeffts[i] = glm::vec4(0.0f);
	block.nLights = 0;

	glGenBuffers(1, &block_ubo);
	glBindBufferRange(GL_UNIFORM_BUFFER, Shader::getUBlockBindingIndex("SHBlock"), block_ubo, 0, sizeof(block));
	glBufferData(GL_UNIFORM_BUFFER, sizeof(block), &(block), GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


SHLight* SHLightManager::add(SHLight* l)
{
	/* Check light to be added is valid, not already in some manager */
	if(block.nLights >= GC::maxSHLights || l == nullptr 
		|| l->manager != nullptr || l->index != -1) return nullptr;
	lights[block.nLights] = l;
	/* Add light's data to uniform buffers */
	for(int c = 0; c < GC::nSHCoeffts; ++c)
		block.lightCoeffts[block.nLights*GC::nSHCoeffts + c] = l->getCoeffts()[c];
	l->index = block.nLights;
	l->manager = this;
	++block.nLights;
	updateBlock();
	return l;
}

SHLight* SHLightManager::update(SHLight* l)
{
	/* Check light is actually in manager before updating */
	if(l->manager != this || l == nullptr) return nullptr;
	/* Check light's index is valid */
	if(l != lights[l->index] ||
		l->index < 0 || l->index >= block.nLights)
		return nullptr; //TODO: throw exception?
	/* Update values in stored buffers */
	for(int c = 0; c < GC::nSHCoeffts; ++c)
		block.lightCoeffts[block.nLights*GC::nSHCoeffts + c] = l->getCoeffts()[c];
	updateBlock();
	return l;
}

SHLight* SHLightManager::remove(SHLight* l)
{
	/* Check light is in manager first */
	if(l->manager != this || l == nullptr) return nullptr;
	/* Check light's index is valid */
	if(l != lights[l->index] ||
		l->index < 0 || l->index >= block.nLights)
		return nullptr; //TODO: throw exception ?
	/* Shift lights to fill gap left by removed light */
	for(int i = l->index; i < block.nLights-1; ++i)
	{
		lights[i] = lights[i+1];
		for(int c = 0; c < GC::nSHCoeffts; ++c)
			block.lightCoeffts[i*GC::nSHCoeffts + c] = block.lightCoeffts[(i+1)*GC::nSHCoeffts + c];
	}
	lights[block.nLights-1] = nullptr;
	for(int c = 0; c < GC::nSHCoeffts; ++c)
		block.lightCoeffts[(block.nLights)*GC::nSHCoeffts + c] = glm::vec4(0.0f);
	--block.nLights;
	l->index = -1;
	l->manager = nullptr;
	updateBlock();
	return l;
}

void SHLightManager::updateBlock()
{
	glBindBuffer(GL_UNIFORM_BUFFER, block_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(block), &(block));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
