#ifndef LIGHTMANAGER_HPP
#define LIGHTMANAGER_HPP

#include <GL/glew.h>
#include <array>
#include <set>

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
	glm::vec4 lightCoeffts[GC::nSHCoeffts];
};

/* LightManager classes, designed to maintain data and 
 * uniform blocks relating to lights in the scene.
 * Each Scene object owns one of each type of LightManager.
 */

class PhongLightManager
{
public:
	PhongLightManager();
	PhongLight* add(PhongLight* l);
	PhongLight* update(PhongLight* l);
	PhongLight* remove(PhongLight* l);
	void updateBlock();
private:
	std::array<PhongLight*, GC::maxPhongLights> lights;
	phongBlock block;
	GLuint block_ubo;
	int nLights;
};

class SHLightManager
{
public:
	SHLightManager();
	SHLight* add(SHLight* l);
	void update();
	SHLight* remove(SHLight* l);
private:
	std::set<SHLight*> lights;
	SHBlock block;
	GLuint block_ubo;
};

#endif
