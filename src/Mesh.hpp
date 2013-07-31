#ifndef MESH_H
#define MESH_H

#include "GC.hpp"
#include "SH.hpp"
#include "Renderable.hpp"
#include "Intersect.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GL/glew.h>

#include <string>
#include <vector>
#include <exception>
#include <fstream>

class Solid;
class Shader;
class LightShader;
class SHSHader;

namespace
{
	struct MeshData
	{
		std::vector<glm::vec4> v;
		std::vector<glm::vec3> n;
		std::vector<GLushort> e;
	};

	struct MeshVertex
	{
		glm::vec4 v;
		glm::vec3 n;
		GLfloat f;
	};

	struct PRTMeshVertex
	{
		glm::vec4 v;
		glm::vec4 s[GC::nSHCoeffts];
	};

	struct AOMeshVertex
	{
		glm::vec4 v;
		glm::vec3 bentN;
		GLfloat occl;
	};

	std::vector<MeshData> loadFileData(const std::string& filename);

	bool isNAN(float f);
	bool isNAN(glm::vec3 v);
}

enum DiffPRTMode : char {UNSHADOWED, SHADOWED, INTERREFLECTED};

class BadPRTModeException : public std::exception {};

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
	GLuint e_vbo;
	GLuint v_attrib;
	GLuint n_attrib;
};

class DiffPRTMesh : public Solid
{
public:
	static std::vector<DiffPRTMesh*> loadFile(
		DiffPRTMode mode,
		const std::string& filename,
		SHShader* _shader);
	void render();
	void update(int dTime) {};
private:
	DiffPRTMesh(const std::vector<PRTMeshVertex>& vertBuffer,
		const std::vector<GLushort>& elemBuffer, SHShader* _shader);
	static std::vector<PRTMeshVertex> computeVertBuffer(
		const MeshData& d, DiffPRTMode mode);

	size_t numElems;
	GLuint v_vbo;
	GLuint e_vbo;
	GLuint v_attrib;
	GLuint s_attrib;
};

class AOMesh : public Solid
{
public:
	static std::vector<AOMesh*> loadFile(
		const std::string& filename,
		AOShader* _shader);
	void render();
	void update(int dTime) {};
private:
	AOMesh(const std::vector<AOMeshVertex>& vertBuffer,
		const std::vector<GLushort>& elemBuffer,
		AOShader* _shader);
	static std::vector<AOMeshVertex> computeVertBuffer(
		const MeshData& d);

	size_t numElems;

	GLuint v_vbo;
	GLuint e_vbo;
	GLuint v_attrib;
	GLuint bentN_attrib;
	GLuint occl_attrib;
};

#endif
