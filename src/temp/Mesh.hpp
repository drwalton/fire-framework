#ifndef MESH_HPP
#define MESH_HPP

struct MeshData
{
	std::vector<glm::vec4> v; //Vertices
	std::vector<glm::vec3> n; //Norms
	std::vector<GLushort > e; //Element indices
	std::vector<Material > M; //Materials
	std::vector<int      > m; //Material indices
};

struct MeshVertex
{
	glm::vec4 v; //Position
	glm::vec3 n; //Norm
	int       m; //Material index
};

class Mesh : public Renderable
{
	Mesh(const std::string& filename, 
		const Material& mat,
		LightShader* shader);
	Mesh(const std::vector<std::string>& filename, 
		const std::vector<Material>& mat,
		LightShader* shader);
	void render();
	void update(int dTime) {};

	static MeshData loadSceneFile(
		const std::string& filename, const Material& mat);

	static MeshData loadSceneFiles(
		const std::vector<std::string>& filenames, 
		const std::vector<Material>& mats);

	static MeshData combineData(
		const std::vector<MeshData>& data);
private:
	void init(const MeshData& data);

	size_t numElems;

	std::vector<Material> mats;
	std::vector<GLushort> matIndices;

	GLuint v_vbo;
	GLuint e_vbo;
	GLuint v_attrib;
	GLuint n_attrib;
	GLuint m_attrib;
};

#endif
