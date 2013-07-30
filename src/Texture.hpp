#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <GL/glew.h>

#include "SOIL.h"
#include <string>

/* Texture
 * Wraps the loading of an image from a file,
 * and importing it as an OpenGL texture.
 */
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
