#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <GL/glew.h>
#include <glm.hpp>

#include "SOIL.h"
#include <string>
#include <exception>
#include <fstream>

class NoImageFileException : public std::exception {};
class BadImageDimException : public std::exception {};

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
	const std::string filename;
private:
	GLuint texUnit;
	GLuint id;
	static GLuint nextTexUnit;
};

enum PixelChannel : int {RED = 0, GREEN = 1, BLUE = 2, ALPHA = 3};
enum ImageFormat : char {TGA, BMP, DDS};

/* Little wrapper around SOIL's image loading.
 *
 */
class Image
{
public:
	Image(const std::string& filename);
	Image(int width, int height);
	~Image();

	void save(const std::string& filename, ImageFormat format = TGA);

	const int getWidth()  const {return width; };
	const int getHeight() const {return height;};
	int getChannels() {return channels;};

	unsigned char getPixel(int u, int v, PixelChannel channel);
	glm::vec4 getPixel(int u, int v);
	const glm::vec4 getPixel(int u, int v) const;
	glm::vec4 getPixel(float u, float v); 
	void setByte(int u, int v, PixelChannel channel, unsigned char byte);
	void setPixel(int u, int v, const glm::vec4& pixel);

private:
	int width; int height; int channels;

	unsigned char* data;
};

#endif
