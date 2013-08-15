#ifndef PRTMESH_HPP
#define PRTMESH_HPP

#include <omp.h>

#include "Mesh.hpp"
#include "Intersect.hpp"
#include "SH.hpp"

enum PRTMode : char {UNSHADOWED, SHADOWED, INTERREFLECTED};

struct PRTMeshVertex
{
	glm::vec4 v; //Postion
	glm::vec2 t; //Texture coord
};

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
		const std::string& coarseMeshFilename,
		const std::string& fineMeshFilename,
		const std::string& diffTex,
		int sqrtNSamples,
		int nBands,
		int nBounces = 3,
		TexCoordGenMode texMode = DONOTGEN);

	void render();
	void update(int dTime) {};
	Shader* getShader() {return shader;};
private:
	static std::string genPrebakedFilename(
		const std::string& filename,
		PRTMode mode,
		int nBands
		);

	void readPrebakedFile(
		std::vector<PRTMeshVertex>& mesh,
		std::vector<GLushort>& elems,
		std::vector<std::string>& coefftFilenames,
	 	const std::string& filename);

	static void interreflect(
		const MeshData& coarseData, const MeshData& fineData,
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

	GLuint v_vbo;
	GLuint e_ebo;
	GLuint v_attrib;
	GLuint t_attrib;
};

#endif
