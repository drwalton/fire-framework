#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <exception>
#include <float.h>
#include <omp.h>

#include <glm.hpp>
#include <GL/glew.h>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include "Scene.hpp"
#include "Texture.hpp"
#include "Intersect.hpp"

bool fileExists(const std::string& filename);

enum TexCoordGenMode : char {CYLINDRICAL, DONOTGEN};

class MeshFileException : public std::exception {};

struct MeshData
{
	std::vector<glm::vec4> v;
	std::vector<glm::vec3> n;
	std::vector<glm::vec2> t;
	std::vector<GLushort > e;
};

struct MeshVertex
{
	glm::vec4 v; // Position
	glm::vec3 n; // Norm 
	glm::vec2 t; // Tex coord
};

class Mesh : public Renderable
{
public:
	Mesh(
		const std::string& meshFilename,
		Texture* ambTex,
		Texture* diffTex,
		Texture* specTex,
		float exponent,
		LightShader* shader,
		TexCoordGenMode mode = DONOTGEN);

	void render();
	void update(int dTime) {};
	Shader* getShader() {return shader;};

	static MeshData loadSceneFile(
		const std::string& filename,
		TexCoordGenMode mode = DONOTGEN);

	static MeshData combineData(const std::vector<MeshData>& data);

	static void genTexCoords(MeshData& data, TexCoordGenMode mode);
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

	static void genTexCoordsCylindrical(MeshData& data);
};

#endif
