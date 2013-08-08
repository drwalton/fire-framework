#include "Texture.hpp"

#include <iostream>

GLuint Texture::nextTexUnit = 0;

Texture::Texture(const std::string& filename)
	:filename(filename)
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

Image::Image(const std::string& filename)
{
	{
		std::ifstream file(filename);
		if(! file)
			throw(new NoImageFileException);
		file.close();
	}

	data = SOIL_load_image
		(
			filename.c_str(),
			&width, &height, &channels,
			SOIL_LOAD_AUTO
		);
}

Image::Image(int width, int height)
	:width(width), height(height), channels(4)
{
	data = static_cast<unsigned char*>(
		malloc(width * height * channels * sizeof(unsigned char)));

	for(int i = 0; i < width; ++i)
		for(int j = 0; j < height; ++j)
		{
			data[(i*channels) + (j*channels*width) + 0] = static_cast<unsigned char>(0);
			data[(i*channels) + (j*channels*width) + 1] = static_cast<unsigned char>(0);
			data[(i*channels) + (j*channels*width) + 2] = static_cast<unsigned char>(0);
			data[(i*channels) + (j*channels*width) + 3] = static_cast<unsigned char>(255);
		}
}

Image::~Image()
{
	if(data) delete data;
}

void Image::save(const std::string& filename, ImageFormat format)
{
	int soilFormat;
	switch(format)
	{
	case TGA:
		soilFormat = SOIL_SAVE_TYPE_TGA;
	case BMP:
		soilFormat = SOIL_SAVE_TYPE_BMP;
	case DDS:
		soilFormat = SOIL_SAVE_TYPE_DDS;
	}

	SOIL_save_image
		(
			filename.c_str(),
			soilFormat,
			width, height, channels,
			data
		);
}

unsigned char Image::getPixel(int u, int v, PixelChannel channel)
{
	if(u < 0 || v < 0 || u >= width || v >= height)
		throw(new BadImageDimException);

	return data[(u*channels) + (v*width*channels) + channel];
}

glm::vec4 Image::getPixel(int u, int v)
{
	if(u < 0 || v < 0 || u >= width || v >= height)
		throw(new BadImageDimException);

	glm::vec4 pixel(0.0f);

	if(channels >= 1)
		pixel.x = static_cast<float>(data[(u*channels) + (v*width*channels) + 0]) / 255.0f;
	if(channels >= 2)
		pixel.y = static_cast<float>(data[(u*channels) + (v*width*channels) + 1]) / 255.0f;
	if(channels >= 3)
		pixel.z = static_cast<float>(data[(u*channels) + (v*width*channels) + 2]) / 255.0f;
	if(channels >= 4)
		pixel.w = static_cast<float>(data[(u*channels) + (v*width*channels) + 3]) / 255.0f;

	return pixel;
}

glm::vec4 Image::getPixel(float u, float v)
{
	/* No filtering. */
	int u_i = static_cast<int>(u * width );
	int v_i = static_cast<int>(v * height);

	if(u < 0 || v < 0 || u >= width || v >= height)
		throw(new BadImageDimException);

	glm::vec4 pixel(0.0f);

	if(channels >= 1)
		pixel.x = static_cast<float>(data[(u_i*channels) + (v_i*width*channels) + 0]) / 255.0f;
	if(channels >= 2)
		pixel.y = static_cast<float>(data[(u_i*channels) + (v_i*width*channels) + 1]) / 255.0f;
	if(channels >= 3)
		pixel.z = static_cast<float>(data[(u_i*channels) + (v_i*width*channels) + 2]) / 255.0f;
	if(channels >= 4)
		pixel.w = static_cast<float>(data[(u_i*channels) + (v_i*width*channels) + 3]) / 255.0f;

	return pixel;
}

const glm::vec4 Image::getPixel(int u, int v) const
{
	if(u < 0 || v < 0 || u >= width || v >= height)
		throw(new BadImageDimException);

	glm::vec4 pixel(0.0f);

	if(channels >= 1)
		pixel.x = static_cast<float>(data[(u*channels) + (v*width*channels) + 0]) / 255.0f;
	if(channels >= 2)
		pixel.y = static_cast<float>(data[(u*channels) + (v*width*channels) + 1]) / 255.0f;
	if(channels >= 3)
		pixel.z = static_cast<float>(data[(u*channels) + (v*width*channels) + 2]) / 255.0f;
	if(channels >= 4)
		pixel.w = static_cast<float>(data[(u*channels) + (v*width*channels) + 3]) / 255.0f;

	return pixel;
}

void Image::setByte(int u, int v, PixelChannel channel, unsigned char byte)
{
	if(u < 0 || v < 0 || u >= width || v >= height)
		throw(new BadImageDimException);

	else 
		data[(u*channels) + (v*width*channels) + channel] = byte;
}

void Image::setPixel(int u, int v, const glm::vec4& pixel)
{
	if(u < 0 || v < 0 || u >= width || v >= height)
		throw(new BadImageDimException);

	if(channels >= 1)
		data[(u*channels) + (v*width*channels) + 0] = 
			static_cast<unsigned char>(pixel.x * 255.0f);

	if(channels >= 2)
		data[(u*channels) + (v*width*channels) + 1] = 
			static_cast<unsigned char>(pixel.y * 255.0f);

	if(channels >= 3)
		data[(u*channels) + (v*width*channels) + 2] = 
			static_cast<unsigned char>(pixel.z * 255.0f);

	if(channels >= 4)
		data[(u*channels) + (v*width*channels) + 3] = 
			static_cast<unsigned char>(pixel.w * 255.0f);
}
