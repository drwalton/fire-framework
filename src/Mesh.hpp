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
#include <omp.h>

#include <string>
#include <vector>
#include <exception>
#include <fstream>

class Solid;
class Shader;
class LightShader;
class SHSHader;

enum MeshLoadMode : char {SEPARATE, COMBINED};

namespace
{
	struct MeshData
	{
		std::vector<glm::vec4> v; //Vertex positions
		std::vector<glm::vec3> n; //Normals
		std::vector<GLushort>  e; //Element indices
		std::vector<GLuint>    m; //Material indices
		std::vector<Material>  M; //Materials
	};

	struct MeshVertex
	{
		glm::vec4 v; //Position
		glm::vec3 n; //Norm
		GLfloat   f; //Padding
		unsigned  m; //Material index
	};

	struct PRTMeshVertex
	{
		glm::vec4                 v; //Position
		glm::vec4 s[GC::nSHCoeffts]; //Transfer func. coeffts
	};

	struct AOMeshVertex
	{
		glm::vec4     v; //Position
		glm::vec3 bentN; //Bent normal
		GLfloat    occl; //Occlusion coefft
	};

	std::vector<MeshData> loadFileData(
		const std::string& filename, MeshLoadMode mode);
	std::vector<MeshData> combineMeshData(const std::vector<MeshData>& data);
	bool fileExists(const std::string& filename);

	bool isNAN(float f);
	bool isNAN(glm::vec3 v);
}

enum DiffPRTMode : char {UNSHADOWED, SHADOWED, INTERREFLECTED, NONE};

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
	static std::vector<Mesh*> loadFile(
		const std::string& filename, MeshLoadMode mode, LightShader* _shader);
	void render();
	void update(int dTime) {};
private:
	Mesh(const MeshData& d, LightShader* _shader,
		const std::vector<Material>& _materials);

	size_t numElems;
	GLuint v_vbo;
	GLuint e_vbo;
	GLuint v_attrib;
	GLuint n_attrib;
	GLuint m_attrib;
};

class DiffPRTMesh : public Solid
{
public:
	static std::vector<DiffPRTMesh*> loadFile(
		DiffPRTMode PRTmode,
		MeshLoadMode loadMode,
		const std::string& filename,
		SHShader* _shader);
	void render();
	void update(int dTime) {};
private:
	DiffPRTMesh(const std::vector<PRTMeshVertex>& vertBuffer,
		const std::vector<GLushort>& elemBuffer, SHShader* _shader);
	static std::vector<PRTMeshVertex> computeVertBuffer(
		const MeshData& d, DiffPRTMode mode);
	static void performInterreflectionPass(
		std::vector<PRTMeshVertex>& vertBuffer,
		const MeshData& d);
	static void writeMeshToFile(
		std::ofstream& file,
		const std::vector<PRTMeshVertex>& vertBuffer,
		unsigned m,
		const std::vector<MeshData>& data
		);

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
		MeshLoadMode mode,
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
