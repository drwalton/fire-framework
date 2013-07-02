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
	Shader(bool hasGeomShader, const std::string& filename,
		bool hasAmbLight, bool hasDirLights, bool hasPointLights);
	void use();
	void setWorldToCamera(const glm::mat4& _worldToCamera);
	void setModelToWorld(const glm::mat4& _modelToWorld);
	GLuint getAttribLoc(const std::string& name);
	//Determines if shader ignores calls to set certain light values.
	bool hasAmbLight; bool hasDirLights; bool hasPointLights;
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
	LightShader(bool hasGeomShader, const std::string& filename,
		bool _hasAmbLight, bool _hasDirLights, bool _hasPointLights);
	void setAmbLight(float _ambLight);
	void setDirLights(GLuint* dirLightOn, glm::vec3* dirLightDir, 
		float* dirIntensity, int nDirLights);
	void setPointLights(GLuint* pointLightOn, 
		glm::vec4* pointLightPos, float* pointIntensity, int nPointLights);
private:
	int nDirLights;
	int nPointLights;

	GLuint ambLight_u;

	GLuint dirLights_u;
	GLuint dirLightOn_u;
	GLuint dirLightDir_u;
	GLuint dirIntensity_u;

	GLuint pointLights_u;
	GLuint pointLightOn_u;
	GLuint pointLightPos_u;
	GLuint pointIntensity_u;
};

#endif
