#include "Texture.hpp"

#include <iostream>

GLuint Texture::nextTexUnit = 0;

Texture::Texture(const std::string& filename)
{
	texUnit = nextTexUnit;
	std::cout << "Texture unit " << texUnit << " created\n";
	++nextTexUnit;

	glActiveTexture(GL_TEXTURE0 + texUnit);

	id = SOIL_load_OGL_texture
	(		 
		filename.c_str(),
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
	);
	if(id == 0) std::cout << "Texture file \"" << filename << "\" could not be loaded.\n";
}

Texture::~Texture()
{
	glDeleteTextures(1, &id);
}

GLuint Texture::getTexUnit()
{
	return texUnit;
}
