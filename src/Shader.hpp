#ifndef SHADER_HPP
#define SHADER_HPP

#include "glsw.h"

#include <glew.h>
#include <glm.hpp>

#include <string>
#include <array>
#include <iostream>

/* Shader
 * Handles opening & compiling glsl source from a file, and setting uniforms.
 */
class Shader
{
public:
	Shader(bool hasGeomShader, const std::string& filename);
	void use();
	void setWorldToCamera(const glm::mat4& _worldToCamera);
	void setModelToWorld(const glm::mat4& _modelToWorld);
	GLuint getAttribLoc(const std::string& name);

	virtual void setAmbLight(const glm::vec4& _ambLight) {};
	virtual void setPhongLights(glm::vec4* pos, glm::vec4* diffuse, 
		glm::vec4* ambient, float* attenuation) {};
	virtual void setMaterial(const Material& material) {};

protected:
	GLuint getUniformLoc(const std::string& name);
private:
	GLuint id;
	GLuint loadShader(const std::string& filename, int shaderType, bool DEBUG);
	GLuint compileShader(const std::string& filename, bool hasGeomShader, bool DEBUG);
	GLuint worldToCamera_u;
	GLuint modelToWorld_u;
};

/* ParticleShader
 * A Shader with additional setters for uniforms related to particle systems.
 * Designed to handle shaders for the AdvectParticle class.
 */
class ParticleShader : public Shader
{
public:
	ParticleShader(bool hasGeomShader, const std::string& filename);
	void setBBWidth(float _bbWidth);
	void setBBHeight(float _bbHeight);
	void setCameraDir(const glm::vec3& _cameraDir);
	void setBBTexUnit(GLuint _bbTexUnit);
	void setDecayTexUnit(GLuint _decayTexUnit);
private:
	GLuint bbWidth_u;
	GLuint bbHeight_u;
	GLuint cameraDir_u;
	GLuint bbTex_u;
	GLuint decayTex_u;
};

/* LightShader
 * A Shader with additional setters for uniforms related to light sources.
 * Designed for objects illuminated by ambient, point and directional lights (e.g. Solid)
 */
class LightShader : public Shader
{
public:
	LightShader(bool hasGeomShader, const std::string& filename);
	LightShader(bool hasGeomShader, const std::string& filename);
	void setAmbLight(const glm::vec4& _ambLight);
	void setPhongLights(glm::vec4* pos, glm::vec4* diffuse, 
		glm::vec4* specular, float* attenuation);
	void setMaterial(const Material& material);
private:
	void init();

	int maxPhongLights;

	GLuint ambLight_u;

	GLuint lightPos_u;
	GLuint lightDiffuse_u;
	GLuint lightSpecular_u;
	GLuint lightAttenuation_u;

	GLuint material_ambient_u;
	GLuint material_diffuse_u;
	GLuint material_specular_u;
	GLuint material_exponent_u;
};

class SHShader : public Shader
{
public:
	SHShader(bool hasGeomShader, int _nSHLights, int _nCoeffts, const std::string& filename);
	void setSHLights(GLuint* SHLightOn, float* SHLights, float* SHIntensity, int nSHLights);
private:
	int nSHLights;
	int nCoeffts;
	GLuint nSHLights_u;
	GLuint nCoeffts_u;

	GLuint SHLights_u;
	GLuint SHLightOn_u;
	GLuint SHIntensity_u;
};

#endif
