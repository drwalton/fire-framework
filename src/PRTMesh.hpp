#ifndef PRTMESH_HPP
#define PRTMESH_HPP

#include "Renderable.hpp"

#include <glm.hpp>
#include <GL/glew.h>

#include <string>
#include <vector>

#include "Shader.hpp"

class ArrayTexture;

enum PRTMode : char {UNSHADOWED, SHADOWED, INTERREFLECTED};

struct PRTMeshVertex
{
	glm::vec4 v; //Postion
	glm::vec2 t; //Texture coord
};

struct MeshData;

/* PRTMesh
 * Class representing an object rendered using
 * diffuse PRT. 
 * Intended to be used by first calling bake() to
 * create a pre-baked file, and then loading this
 * via the constructor to create PRTMesh objects.
 */
class PRTMesh : public Renderable
{
public:
	PRTMesh(
		const std::string& bakedFilename,
		SHShader* shader);
	~PRTMesh();

	static void bake(
		PRTMode mode,
		const std::string& meshFilename,
		const std::string& bakedFilename,
		const std::string& diffTex,
		int sqrtNSamples,
		int nBands,
		int nBounces = 3);

	void render();
	void update(int dTime) {};
	Shader* getShader() {return static_cast<Shader*>(shader);};
private:
	static std::string genExt(PRTMode mode, int nBands);

	void readPrebakedFile(
		std::vector<PRTMeshVertex>& mesh,
		std::vector<GLushort>& elems,
		std::vector<std::string>& coefftFilenames,
	 	const std::string& filename);

	static void interreflect(
		const MeshData& data,
		const std::string& diffTex,
		int nBands, int sqrtNSamples, int nBounces,
		std::vector<std::vector<glm::vec3>>& transfer);

	static void writePrebakedFile(
		const std::vector<PRTMeshVertex>& mesh,
		const std::vector<GLushort>& elems,
		const std::vector<std::string>& coefftTex,
		const std::string& filename);

	static void renderCoefftToTexture(
		const std::vector<glm::vec3>& coefft,
		const std::string& image,
		const MeshData& data,
		int width, int height);

	static glm::vec3 texLookup(
		unsigned char* image, 
		const glm::vec2& uv,
		int width, int height, int channels);

	static void writeTransferToTextures(
		const std::vector<std::vector<glm::vec3>>& transfer,
		const MeshData& data,
		const std::string& prebakedFilename,
		std::vector<std::string>& coefftFilenames,
		int width, int height);

	void init(
		const std::vector<PRTMeshVertex>& mesh,
		const std::vector<GLushort>& elems);

	SHShader* shader;
	size_t numElems;

	ArrayTexture* arrTex;

	GLuint vao;
	GLuint v_vbo;
	GLuint e_ebo;
	GLuint v_attrib;
	GLuint t_attrib;
};

#endif
