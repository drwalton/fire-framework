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

struct SHBlock
{
	glm::vec4 lightCoeffts[GC::nSHCoeffts * GC::maxSHLights];
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
	SHLight* update(SHLight* l);
	SHLight* remove(SHLight* l);
private:
	std::array<SHLight*, GC::maxSHLights> lights;
	SHBlock block;
	GLuint block_ubo;
	void updateBlock();
	int nLights;
};

#endif
