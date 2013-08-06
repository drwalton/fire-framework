#ifndef AOMESH_HPP
#define AOMESH_HPP

#include "Mesh.hpp"
#include "Intersect.hpp"

struct AOMeshVertex
{
	glm::vec4  v; //Position
	glm::vec3  n; //Bent normal
	GLuint     m; //Material index
	GLfloat occl; //Occlusion coefft
};

class AOMesh : public Renderable
{
public:
	AOMesh(
		const std::string& filenames,
		const Material& materials,
		AOShader* shader, int sqrtNSamples);
	AOMesh(
		const std::vector<std::string>& filenames,
		const std::vector<Material>& materials,
		AOShader* shader, int sqrtNSamples);
private:
	void writePrebakedFile(
		const std::vector<AOMeshVertex>& mesh,
		const std::vector<GLushort>& elems,
	 	const std::vector<Material>& mats,
	 	const std::string& filename);
	void readPrebakedFile(
		std::vector<AOMeshVertex>& mesh,
		std::vector<GLushort>& elems,
	 	std::vector<Material>& mats,
	 	const std::string& filename);
	void init(
		const std::vector<AOMeshVertex>& mesh,
		const std::vector<GLushort>& elems);
	void bake(
		const MeshData& data,
		int sqrtNSamples,
		std::vector<AOMeshVertex>& mesh, 
		const std::vector<GLushort>& elems);
	void render();
	void update(int dTime) {};

	LightShader* shader;

	size_t numElems;

	std::vector<Material> mats;

	GLuint v_vbo;
	GLuint e_vbo;
	GLuint v_attrib;
	GLuint n_attrib;
	GLuint m_attrib;
	GLuint occl_attrib;
};

#endif
