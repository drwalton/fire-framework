#ifndef SHADER_HPP
#define SHADER_HPP

#include <glew.h>
#include <glm.hpp>
#include <boost\algorithm\string.hpp>

#include "glsw.h"
#include "GC.hpp"

#include <string>
#include <iostream>
#include <exception>
#include <vector>

struct Material
{
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	float exponent;
};

/* Shader
 * Handles opening & compiling glsl source from a file, and setting uniforms.
 */
class Shader
{
public:
	Shader(bool hasGeomShader, const std::string& filename);
	Shader(bool hasGeomShader, const std::string& filename,
		std::vector<std::string> subs);
	void use();
	void setModelToWorld(const glm::mat4& _modelToWorld);
	GLuint getAttribLoc(const std::string& name);

	virtual void setMaterial(const Material& material) {};

	const std::string filename;
	static GLuint getUBlockBindingIndex(const std::string& name);
protected:
	GLuint getUniformLoc(const std::string& name);
	void setupUniformBlock(const std::string& name);

	static const std::vector<std::string> PHONG_SUBS;
	static const std::vector<std::string> SH_SUBS;
private:
	GLuint id;
	GLuint loadShader(const std::string& filename, int shaderType, bool DEBUG,
		std::vector<std::string> subs);
	GLuint compileShader(const std::string& filename, bool hasGeomShader, bool DEBUG,
		std::vector<std::string> subs);
	GLuint modelToWorld_u;
	GLuint cameraBlock_i;
};

/* Error classes for Shader */
class NoSuchException : public std::exception 
{
public:
	NoSuchException(const std::string& name, Shader* const& shader);
private:
	std::string name;
};
class NoSuchUniformException : public NoSuchException 
{
public:
	NoSuchUniformException(const std::string& name, Shader* const& shader) 
		:NoSuchException(name, shader) {};
};
class NoSuchAttribException : public NoSuchException 
{
public:
	NoSuchAttribException(const std::string& name, Shader* const& shader) 
		:NoSuchException(name, shader) {};
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
	void setBBTexUnit(GLuint _bbTexUnit);
	void setDecayTexUnit(GLuint _decayTexUnit);
private:
	GLuint bbWidth_u;
	GLuint bbHeight_u;
	GLuint bbTex_u;
	GLuint decayTex_u;
};

/* LightShader
 * A Shader with additional setters for uniforms related to light sources.
 * Designed for objects illuminated by ambient, point and directional lights
 * (e.g. Solid)
 */
class LightShader : public Shader
{
public:
	LightShader(bool hasGeomShader, const std::string& filename);
	LightShader(bool hasGeomShader, const std::string& filename,
		std::vector<std::string> subs);

	void setMaterial(const Material& material);
	void setWorldToCamera(const glm::mat4& _worldToCamera);
private:
	void init();

	int maxPhongLights;

	GLuint ambBlock_i;

	GLuint material_ambient_u;
	GLuint material_diffuse_u;
	GLuint material_specular_u;
	GLuint material_exponent_u;
};

class SHShader : public Shader
{
public:
	SHShader(bool hasGeomShader, const std::string& filename);
	SHShader(bool hasGeomShader, const std::string& filename,
		std::vector<std::string> subs);
};

class AOShader : public LightShader
{
public:
	AOShader(bool hasGeomShader, const std::string& filename);
	AOShader(bool hasGeomShader, const std::string& filename,
		std::vector<std::string> subs);
};

#endif
