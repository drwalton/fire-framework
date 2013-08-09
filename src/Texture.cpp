#include "Texture.hpp"

#include <iostream>

GLuint Texture::nextTexUnit = 0;

Texture::Texture(const std::string& filename)
	:filename(filename)
{
	texUnit = genTexUnit();

	glActiveTexture(GL_TEXTURE0 + texUnit);

	id = SOIL_load_OGL_texture
	(		 
		filename.c_str(),
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
	);
	if(id == 0) throw(new NoImageFileException);
}

Texture::~Texture()
{
	glDeleteTextures(1, &id);
}

GLuint Texture::getTexUnit()
{
	return texUnit;
}

GLuint Texture::genTexUnit()
{
	GLuint unit = nextTexUnit;
	nextTexUnit++;
	return unit;
}

ArrayTexture::ArrayTexture(const std::vector<std::string>& filenames)
	:filenames(filenames)
{
	nLayers = filenames.size();

	texUnit = Texture::genTexUnit();
	glActiveTexture(GL_TEXTURE0 + texUnit);

	int width, height, channels;

	std::vector<unsigned char> compiledImages;

	for(auto f = filenames.begin(); f != filenames.end(); ++f)
	{
		unsigned char* fileData = SOIL_load_image
			(
				f->c_str(),
				&width, &height, &channels,
				SOIL_LOAD_AUTO
			);

		for(int u = 0; u < width; ++u)
			for(int v = 0; v < height; ++v)
			{
				compiledImages.push_back(fileData[(u + v*height)*channels  ]);
				compiledImages.push_back(fileData[(u + v*height)*channels+1]);
				compiledImages.push_back(fileData[(u + v*height)*channels+2]);
			}

		SOIL_free_image_data(fileData);
	}

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D_ARRAY, id);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGB8, width, height, nLayers);
	glTexSubImage3D(
		GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, 
		width, height, nLayers, 
		GL_RGB, GL_UNSIGNED_BYTE, 
		compiledImages.data());

	glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
}

ArrayTexture::~ArrayTexture()
{
	glDeleteTextures(1, &id);
}
