#ifndef MESH_H
#define MESH_H

#include "Shader.hpp"
#include "Renderable.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>
#include <exception>


namespace
{
	struct MeshData
	{
		std::vector<glm::vec4> v;
		std::vector<glm::vec3> n;
		std::vector<GLushort> e;
	};

	std::vector<MeshData> loadFileData(const std::string& filename);
}

class MeshFileException : public std::exception {};

/* Mesh
 * Supports loading & rendering of meshes loaded via the AssImp library.
 * In general, a file can contain multiple meshes. Thus, meshes should be 
 * loaded via Mesh::loadFile() which returns a vector of pointers to 
 * the loaded mesh objects.
 * **N.B.** Only the mesh itself (vertices, elements, norms) are loaded. 
 *          Textures, other attributes are ignored. 
 */
class Mesh : public Solid
{
public:
	static std::vector<Mesh*> loadFile(const std::string& filename, 
		LightShader* _shader);
	void render();
	void update(int dTime) {};
private:
	Mesh(const MeshData& d, LightShader* _shader);

	size_t numElems;
	GLuint v_vbo;
	GLuint n_vbo;
	GLuint e_vbo;
	GLuint v_attrib;
	GLuint n_attrib;
	GLuint e_attrib;
};

class DiffPRTMesh : public Solid
{
public:
	static std::vector<DiffPRTMesh*> loadFile(
		const std::string& filename,
		int nBands,
		SHShader* _shader);
	void render();
	void update(int dTime) {};
private:
	DiffPRTMesh(const MeshData& d, int nBands, SHShader* _shader);

	size_t nBands;
	GLuint nBands_u;

	size_t numElems;
	GLuint v_vbo;
	GLuint s_vbo;
	GLuint e_vbo;
	GLuint v_attrib;
	GLuint s_attrib;
	GLuint e_attrib;
};

#endif
