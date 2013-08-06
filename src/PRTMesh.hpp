#ifndef PRTMESH_HPP
#define PRTMESH_HPP

#include "Mesh.hpp"

enum PRTMode : char {UNSHADOWED, SHADOWED, INTERREFLECTED, NONE};

class PRTMesh : public Renderable
{
public:
	PRTMesh(
		const std::string& filename,
		const Material& material,
		PRTMode mode, int sqrtNSamples,
		int nBands,
		Shader* shader);

	PRTMesh(
		const std::vector<std::string>& filenames,
		const std::vector<Material>& materials,
		PRTMode mode, int sqrtNSamples,
		int nBands,
		Shader* shader);

	void render();

	void update(int dTime) {};
private:
	std::string genPrebakedFilename(
		const std::string& filename,
		PRTMode mode,
		int nBands
		);

	void writePrebakedFile(
		const std::vector<glm::vec4>& verts,
		const std::vector<GLushort>& elems,
	 	const std::vector<std::vector<glm::vec3>>& transfer,
	 	const std::string& filename);

	void readPrebakedFile(
		std::vector<glm::vec4>& verts,
		std::vector<GLushort>& elems,
	 	std::vector<std::vector<glm::vec3>>& transfer,
	 	int nCoeffts,
	 	const std::string& filename);

	void init();

	void bake(const MeshData& data,
		PRTMode mode, int nBands, 
		std::vector<glm::vec4>& verts,
		std::vector<std::vector<glm::vec3>>& transfer);

	void interreflect(
		const MeshData& data,
		int nBands, 
		const std::vector<glm::vec4>& verts,
		std::vector<std::vector<glm::vec3>>& transfer);

	Shader* shader;

	std::vector<glm::vec4> verts;
	std::vector<GLushort>  elems;

	std::vector<std::vector<glm::vec3>> transfer;
	std::vector<glm::vec4> colors;

	GLuint verts_vbo;
	GLuint elems_vbo;
	GLuint colors_vbo;

	GLuint vert_attrib;
	GLuint color_attrib;
};

#endif
