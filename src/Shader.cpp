#include "Shader.hpp"

NoSuchException::NoSuchException(const std::string& name, Shader* const& shader)
{
	std::cout << "Could not find name \"" << name 
		<<"\" in shader source file \"" << shader->filename << "\".";
}

Shader::Shader(bool hasGeomShader, const std::string& filename)
	:filename(filename)
{
	id = compileShader(filename, hasGeomShader, true);
	worldToCamera_u = getUniformLoc("worldToCamera");
	modelToWorld_u = getUniformLoc("modelToWorld");
}

void Shader::use()
{
	glUseProgram(id);
}

void Shader::setWorldToCamera(const glm::mat4& worldToCamera)
{
	use();
	glUniformMatrix4fv(worldToCamera_u, 1, GL_FALSE, &(worldToCamera[0][0]));
	glUseProgram(0);
}

void Shader::setModelToWorld(const glm::mat4& modelToWorld)
{
	use();
	glUniformMatrix4fv(modelToWorld_u, 1, GL_FALSE, &(modelToWorld[0][0]));
	glUseProgram(0);
}

GLuint Shader::loadShader(const std::string& filename, int shaderType, bool DEBUG)
{
	glswInit();
	glswSetPath("./", ".glsl");

	const char* source;
	switch(shaderType)
	{
		case GL_VERTEX_SHADER:
			source = glswGetShader((filename + ".Vertex").c_str());
			break;
		case GL_FRAGMENT_SHADER:
			source = glswGetShader((filename + ".Fragment").c_str());
			break;
		case GL_GEOMETRY_SHADER:
			source = glswGetShader((filename + ".Geometry").c_str());
			break;
		default:
			if(DEBUG) std::cout <<"loadShader error: invalid shader type\n";
			return 0;
			break;
	}
	if(!source && DEBUG) 
	{
		std::cout << "File containing " << 
			(
				shaderType == GL_VERTEX_SHADER ? "vertex " :
				shaderType == GL_FRAGMENT_SHADER ? "fragment " :
				"geometry "
			)
			<< "shader could not be found." << std::endl;
		return 0;
	}
	GLuint shaderObject = glCreateShader(shaderType);
	std::cout << shaderObject;
	glShaderSource(shaderObject, 1, &source, 0);
	glCompileShader(shaderObject);

	GLint compiled;
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &compiled);
	if (compiled)
	{
	   if(DEBUG) std::cout <<
			((shaderType == GL_VERTEX_SHADER) ? "Vertex" :
			(shaderType == GL_FRAGMENT_SHADER) ? "Fragment" :
			"Geometry") << " shader compiled successfully.\n";
	   return shaderObject;
	}
	else
	{
		if(DEBUG)
		{
			GLchar messages[256];
			glGetShaderInfoLog(shaderObject, sizeof(messages), 0, &messages[0]);
			std::cout << "Compilation error:\n" << messages;
			std::cout << "Loaded source:\n" << source;
		}
		return 0;
	}
	glswShutdown();
}

GLuint Shader::compileShader(const std::string& filename, bool hasGeomShader, bool DEBUG)
{
	if(DEBUG) std::cout << "Attempting to load shaders from ./" << filename << ".glsl" << std::endl;

	GLuint vertexShader = loadShader(filename, GL_VERTEX_SHADER, DEBUG);
	GLuint fragmentShader = loadShader(filename, GL_FRAGMENT_SHADER, DEBUG);
	GLuint geomShader = 1;
	if(hasGeomShader) geomShader = loadShader(filename, GL_GEOMETRY_SHADER, DEBUG);

	if(!vertexShader || !fragmentShader || !geomShader)
		return 0;

	GLuint program = glCreateProgram();

	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	if(hasGeomShader) glAttachShader(program, geomShader);

	glLinkProgram(program);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	if(hasGeomShader) glDeleteShader(geomShader);

	GLint linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (linked)
	{
	   if(DEBUG) std::cout << "Program linked successfully.\n";
	   return program;
	}
	else
	{
		if(DEBUG) 
		{
			std::cout << "Program linking failed.\n";
			GLchar messages[256];
			glGetProgramInfoLog(program, sizeof(messages), 0, &messages[0]);
			std::cout << "Compilation error:\n" << messages;
		}
		return 0;
	}
}

GLuint Shader::getAttribLoc(const std::string& name)
{
	GLuint loc = glGetAttribLocation(id, name.c_str());
	if (loc == -1) throw new NoSuchAttribException(name, this);
	return loc;
}

GLuint Shader::getUniformLoc(const std::string& name)
{
	GLuint loc = glGetUniformLocation(id, name.c_str());
	if (loc == -1) throw new NoSuchUniformException(name, this);
	return loc;
}

LightShader::LightShader(bool hasGeometry, const std::string& filename)
	:Shader(hasGeometry, filename), maxPhongLights(50)
{
	init();
}

void LightShader::init()
{
	use();
	ambLight_u         = getUniformLoc("ambLight");
	cameraPos_u        = getUniformLoc("cameraPos");
	lightPos_u         = getUniformLoc("lightPos");
	lightDiffuse_u     = getUniformLoc("lightDiffuse");
	lightSpecular_u    = getUniformLoc("lightSpecular");
	lightAttenuation_u = getUniformLoc("lightAttenuation");

	material_ambient_u  = getUniformLoc("material_ambient");
	material_diffuse_u  = getUniformLoc("material_diffuse");
	material_specular_u = getUniformLoc("material_specular");
	material_exponent_u = getUniformLoc("material_exponent");
	glUseProgram(0);
}


void LightShader::setAmbLight(const glm::vec4& _ambLight)
{
	use();
	glUniform4fv(ambLight_u, 1, &(_ambLight[0]));
	glUseProgram(0);
}

void LightShader::setPhongLights(glm::vec4* pos, glm::vec4* diffuse, 
		glm::vec4* specular, float* attenuation)
{
	use();
	glUniform4fv(lightPos_u, maxPhongLights, &(pos[0][0]));
	glUniform4fv(lightDiffuse_u, maxPhongLights, &(diffuse[0][0]));
	glUniform4fv(lightSpecular_u, maxPhongLights, &(specular[0][0]));
	glUniform1fv(lightAttenuation_u, maxPhongLights, &(attenuation[0]));
	glUseProgram(0);
}

void LightShader::setMaterial(const Material& material)
{
	use();
	glUniform4fv(material_ambient_u, 1, &(material.ambient)[0]);
	glUniform4fv(material_diffuse_u, 1, &(material.diffuse[0]));
	glUniform4fv(material_specular_u, 1, &(material.specular[0]));
	glUniform1fv(material_exponent_u, 1, &(material.exponent));
	glUseProgram(0);
}

void LightShader::setWorldToCamera(const glm::mat4& _worldToCamera)
{
	Shader::setWorldToCamera(_worldToCamera);
	use();
	glm::mat4 inv = glm::inverse(_worldToCamera);
	glm::vec3 cameraPos = glm::vec3(inv[3][0], inv[3][1], inv[3][2]);
	glUniform3fv(cameraPos_u, 1, &(cameraPos[0]));
	glUseProgram(0);
}

ParticleShader::ParticleShader(bool hasGeomShader, const std::string& filename)
	:Shader(hasGeomShader, filename)
{
	use();
	bbWidth_u = getUniformLoc("bbWidth");
	bbHeight_u = getUniformLoc("bbHeight");
	cameraDir_u = getUniformLoc("cameraDir");
	bbTex_u = getUniformLoc("bbTexture");
	decayTex_u = getUniformLoc("decayTexture");
	glUseProgram(0);
}

void ParticleShader::setBBWidth(float _bbWidth)
{
	use();
	glUniform1fv(bbWidth_u, 1, &_bbWidth);
	glUseProgram(0);
}

void ParticleShader::setBBHeight(float _bbHeight)
{
	use();
	glUniform1fv(bbHeight_u, 1, &_bbHeight);
	glUseProgram(0);
}

void ParticleShader::setCameraDir(const glm::vec3& _cameraDir)
{
	use();
	glUniform3fv(cameraDir_u, 1, &(_cameraDir.x));
	glUseProgram(0);
}

void ParticleShader::setBBTexUnit(GLuint _bbTexUnit)
{
	use();
	glUniform1i(bbTex_u, _bbTexUnit);
	glUseProgram(0);
}

void ParticleShader::setDecayTexUnit(GLuint _decayTexUnit)
{
	use();
	glUniform1i(decayTex_u, _decayTexUnit);
	glUseProgram(0);
}

SHShader::SHShader(bool hasGeomShader, int _nSHLights, int _nCoeffts, const std::string& filename)
	:Shader(hasGeomShader, filename), nSHLights(_nSHLights), nCoeffts(_nCoeffts)
{
	use();
	nCoeffts_u = getUniformLoc("nCoeffts");
	nSHLights_u = getUniformLoc("nSHLights");
	SHLights_u = getUniformLoc("SHLights");
	SHLightOn_u = getUniformLoc("SHLightOn");
	SHIntensity_u = getUniformLoc("SHIntensity");

	glUniform1i(nSHLights_u, nSHLights);
	glUniform1i(nCoeffts_u, nCoeffts);
	glUseProgram(0);
}

void SHShader::setSHLights(GLuint* SHLightOn, float* SHLights, float* SHIntensity, int nSHLights)
{
	use();
	glUniform1uiv(SHLightOn_u, nSHLights, SHLightOn);
	glUniform1fv(SHLights_u, nSHLights * nCoeffts, SHLights);
	glUniform1fv(SHIntensity_u, nSHLights, SHIntensity);
	glUseProgram(0);
}
