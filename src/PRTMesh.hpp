#ifndef PRTMESH_HPP
#define PRTMESH_HPP

#include <omp.h>

#include "Mesh.hpp"
#include "Intersect.hpp"
#include "SH.hpp"

enum PRTMode : char {UNSHADOWED, SHADOWED, INTERREFLECTED, NONE};

struct PRTMeshVertex
{
	glm::vec4 v; //Postion
	glm::vec3 t; //Texture coord
};

class PRTMesh : public Renderable
{
public:
	PRTMesh(
		const std::string& bakedFilename,
		Shader* shader);

	static void bake(
		PRTMode mode,
		const std::string& meshFilename,
		const std::string& diffTex,
		int sqrtNSamples,
		int nBounces = 3,
		texCoordGenMode = DONOTGEN);

	void render();
	void update(int dTime) {};
	Shader* getShader() {return shader;};
private:
	std::string genPrebakedFilename(
		const std::string& filename,
		PRTMode mode,
		int nBands
		);

	void readPrebakedFile(
		std::vector<glm::vec4>& verts,
		std::vector<GLushort>& elems,
	 	std::vector<std::vector<glm::vec3>>& transfer,
	 	int nCoeffts,
	 	const std::string& filename);

	static void interreflect(
		const MeshData& data,
		int nBands, int sqrtNSamples, int nBounces,
		const std::vector<glm::vec4>& verts,
		std::vector<std::vector<glm::vec3>>& transfer);

	static void writePrebakedFile(
		const std::vector<AOMeshVertex>& mesh,
		const std::vector<GLushort>& elems,
		const std::vector<std::string>& coefftTex
		const std::string& filename);

	static void renderCoefftToImage(
		const std::vector<glm::vec3>& coefft,
		const std::string& image,
		const MeshData& data);

	void init();

	Shader* shader;
	size_t numElems;

	std::vector<Texture> coefftTex;

	GLuint v_vbo;
	GLuint e_ebo;
	GLuint v_attrib;
	GLuint t_attrib;
};

#endif
