#ifndef LIGHTMANAGER_HPP
#define LIGHTMANAGER_HPP

#include <GL/glew.h>
#include <array>

#include "Light.hpp"
#include "GC.hpp"
#include "Shader.hpp"

class PhongLight;
class SHLight;


struct phongBlock
{
	glm::vec4 lightPos[GC::maxPhongLights];
	glm::vec4 lightDiffuse[GC::maxPhongLights];
	glm::vec4 lightSpecular[GC::maxPhongLights];
	float lightAttenuation[GC::maxPhongLights];
};

class PhongLightManager
{
public:
	PhongLightManager();
	PhongLight* add(PhongLight* l);
	PhongLight* update(PhongLight* l);
	PhongLight* remove(PhongLight* l);
private:
	std::array<PhongLight*, GC::maxPhongLights> lights;
	phongBlock block;
	GLuint block_ubo;
	void updateBlock();
	int nLights;
};

class SHLightManager
{
public:
	SHLightManager();
	SHLight* add(SHLight* l);
	SHLight* remove(SHLight* l);
	glm::vec4 getSHLitColor(const std::vector<glm::vec3>& coeffts);
private:
	std::array<SHLight*, GC::maxSHLights> lights;
	int nLights;
};

#endif
