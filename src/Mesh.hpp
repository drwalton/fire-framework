#ifndef MESH_HPP
#define MESH_HPP

#include "Renderable.hpp"
#include "Shader.hpp"

#include <vector>
#include <string>
#include <exception>

#include <glm.hpp>
#include <GL/glew.h>

bool fileExists(const std::string& filename);

class MeshFileException : public std::exception {};

struct MeshData
{
	std::vector<glm::vec4> v;
	std::vector<glm::vec3> n;
	std::vector<glm::vec2> t;
	std::vector<GLushort > e;
};

class Texture;

struct MeshVertex
{
	glm::vec4 v; // Position
	glm::vec3 n; // Norm 
	glm::vec2 t; // Tex coord
};

/* Mesh
 * A Renderable object containing a mesh, loaded from
 * a 3D model file and illuminated using Blinn-Phong 
 * shading with ambient, diffuse and specular textures.
 * Has a rudimentary tex coord generation method, but it
 * is preferable to input meshes with their own texture
 * coordinates.
 */
class Mesh : public Renderable
{
public:
	Mesh(
		const std::string& meshFilename,
		Texture* ambTex,
		Texture* diffTex,
		Texture* specTex,
		float exponent,
		LightShader* shader);

	void render();
	void update(int dTime) {};
	Shader* getShader() {return shader;};

	static MeshData loadSceneFile(
		const std::string& filename);

	static MeshData combineData(const std::vector<MeshData>& data);
private:
	void init(const MeshData& data);

	LightShader* shader;
	size_t numElems;

	Texture* ambTex;
	Texture* diffTex;
	Texture* specTex;
	float specExp;

	GLuint v_vbo;
	GLuint e_vbo;
	GLuint v_attrib;
	GLuint n_attrib;
	GLuint t_attrib;
};

#endif
