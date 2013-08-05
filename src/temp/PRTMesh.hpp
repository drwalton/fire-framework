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
		PRTMode mode, int nBands,
		Shader* shader);

	PRTMesh(
		const std::vector<std::string>& filenames,
		const std::vector<Material>& materials,
		PRTMode mode, int nBands,
		Shader* shader);
private:
	void init(const MeshData& data, PRTMode mode, int nBands);

	Shader* shader;
	size_t numElems;

	std::vector<glm::vec4> vert;
	std::vector<GLushort>  elem;

	std::vector<std::vector<glm::vec3>> transfer;
	std::vector<glm::vec4> outColor;

	GLuint vert_vbo;
	GLuint elem_vbo;
	GLuint outColor_vbo;

	GLuint vert_attrib;
	GLuint color_attrib;
};

#endif
