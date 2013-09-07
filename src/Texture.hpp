#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <GL/glew.h>
#include <glm.hpp>

#include <string>
#include <vector>

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
	static GLuint genTexUnit();
private:
	GLuint texUnit;
	GLuint id;
	static GLuint nextTexUnit;
};

/* ArrayTexture
 * Wraps the loading of a series of 2D textures,
 *   and subsequent conversion into a 2D array texture.
 */
class ArrayTexture
{
public:
	ArrayTexture(const std::vector<std::string>& filenames);
	~ArrayTexture();
	GLuint getTexUnit() {return texUnit;};
	const std::vector<std::string> filenames;
private:
	size_t nLayers;
	GLuint texUnit;
	GLuint id;
};

#endif
