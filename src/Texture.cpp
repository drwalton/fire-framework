#include "Texture.hpp"

GLuint Texture::nextTexUnit = 0;

Texture::Texture(const std::string& filename)
{
	texUnit = nextTexUnit;
	++nextTexUnit;

	glActiveTexture(GL_TEXTURE0 + texUnit);

	id = SOIL_load_OGL_texture
	(		 
		filename.c_str(),
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
	);
}

Texture::~Texture()
{
	glDeleteTextures(1, &id);
}

GLuint Texture::getTexUnit()
{
	return texUnit;
}
