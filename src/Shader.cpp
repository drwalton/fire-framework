#include "Shader.hpp"

Shader::Shader(bool hasGeomShader, const std::string& filename)
	:hasAmbLight(false),
	hasDirLights(false),
	hasPointLights(false)
{
	id = compileShader(filename, hasGeomShader, true);
	worldToCamera_u = getUniformLoc("worldToCamera");
	modelToWorld_u = getUniformLoc("modelToWorld");
}

Shader::Shader(bool hasGeomShader, const std::string& filename,
		bool _hasAmbLight, bool _hasDirLights, bool _hasPointLights)
	:hasAmbLight(_hasAmbLight),
	hasDirLights(_hasDirLights),
	hasPointLights(_hasPointLights)
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
	return glGetAttribLocation(id, name.c_str());
}

GLuint Shader::getUniformLoc(const std::string& name)
{
	return glGetUniformLocation(id, name.c_str());
}

LightShader::LightShader(bool hasGeometry, const std::string& filename,
	bool _hasAmbLight, bool _hasDirLights, bool _hasPointLights)
	:Shader(hasGeometry, filename, 
	_hasAmbLight, _hasDirLights, _hasPointLights)
{
	use();
	if(hasAmbLight)
	{
		ambLight_u = getUniformLoc("ambLight");
	}
	if(hasDirLights)
	{
		dirLightOn_u = getUniformLoc("dirLightOn");
		dirLightDir_u = getUniformLoc("dirLightDir");
		dirIntensity_u = getUniformLoc("dirIntensity");
	}
	if(hasPointLights)
	{
		pointLightOn_u = getUniformLoc("pointLightOn");
		pointLightPos_u = getUniformLoc("pointLightPos");
		pointIntensity_u = getUniformLoc("pointIntensity");
	}
	glUseProgram(0);
}

void LightShader::setAmbLight(float _ambLight)
{
	if(!hasAmbLight) return;
	use();
	glUniform1f(ambLight_u, _ambLight);
	glUseProgram(0);
}

void LightShader::setDirLights(GLuint* dirLightOn, glm::vec3* dirLightDir, 
	float* dirIntensity, int nDirLights)
{
	if(!hasDirLights) return;
	use();
	glUniform1uiv(dirLightOn_u, nDirLights, dirLightOn);
	glUniform3fv(dirLightDir_u, nDirLights, (GLfloat*) dirLightDir);
	glUniform1fv(dirIntensity_u, nDirLights, (GLfloat*) dirIntensity);
	glUseProgram(0);
}

void LightShader::setPointLights(GLuint* pointLightOn, 
	glm::vec4* pointLightPos, float* pointIntensity, int nPointLights)
{
	if(!hasPointLights) return;
	use();
	glUniform1uiv(pointLightOn_u, nPointLights, pointLightOn);
	glUniform4fv(pointLightPos_u, nPointLights, (GLfloat*) pointLightPos);
	glUniform1fv(pointIntensity_u, nPointLights,(GLfloat*) pointIntensity);
	glUseProgram(0);
}

ParticleShader::ParticleShader(bool hasGeomShader, const std::string& filename)
	:Shader(hasGeomShader, filename)
{
	use();
	bbWidth_u = getUniformLoc("bbWidth");
	bbHeight_u = getUniformLoc("bbHeight");
	cameraPos_u = getUniformLoc("cameraPos");
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

void ParticleShader::setCameraPos(const glm::vec3& _cameraPos)
{
	use();
	glUniform3fv(cameraPos_u, 1, &(_cameraPos.x));
	glUseProgram(0);
}

SFlameShader::SFlameShader(bool hasGeomShader, const std::string& filename)
	:Shader(hasGeomShader, filename)
{
	use();
	width_u = getUniformLoc("width");
	height_u = getUniformLoc("height");
	displaceWidth_u = getUniformLoc("displaceWidth");
	time_u = getUniformLoc("time");
	timeScale_u = getUniformLoc("timeScale");
	heightScale_u = getUniformLoc("heightScale");
	glUseProgram(0);
}

void SFlameShader::setWidth(float _width)
{
	use();
	glUniform1fv(width_u, 1, &_width);
	glUseProgram(0);
} 

void SFlameShader::setHeight(float _height)
{
	use();
	glUniform1fv(height_u, 1, &_height);
	glUseProgram(0);
}
void SFlameShader::setDisplaceWidth(float _displaceWidth)
{
	use();
	glUniform1fv(displaceWidth_u, 1, &_displaceWidth);
	glUseProgram(0);
}
void SFlameShader::setTime(float _time)
{
	use();
	glUniform1fv(time_u, 1, &_time);
	glUseProgram(0);
}
void SFlameShader::setTimeScale(float _timeScale)
{
	use();
	glUniform1fv(timeScale_u, 1, &_timeScale);
	glUseProgram(0);
}
void SFlameShader::setHeightScale(float _heightScale)
{
	use();
	glUniform1fv(heightScale_u, 1, &_heightScale);
	glUseProgram(0);
}	
