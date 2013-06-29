#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <glew.h>

#include "SOIL.h"
#include <string>

class Texture
{
public:
	Texture(const std::string& filename);
	~Texture();
	GLuint getTexUnit();
private:
	GLuint texUnit;
	GLuint id;
	static GLuint nextTexUnit;
};

#endif
