#include "Mesh.hpp"

std::vector<Mesh*> Mesh::loadFile(const std::string& filename,
	LightShader* _shader)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filename,
		aiProcess_CalcTangentSpace | aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices | aiProcess_SortByPType );

	if(!scene)
	{
		//TODO: throw exception?
		std::cout << "Model file does not exist or could not be loaded.\n";
		return std::vector<Mesh*>();
	}

	std::cout << "Loading from file: " << filename << "\n" 
		<< scene->mNumMeshes << " meshes found.\n";

	std::vector<Mesh*> meshes;

	for(int i = 0; i < scene->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[i];

		std::vector<glm::vec4> v;
		std::vector<glm::vec3> n;
		std::vector<GLushort> e;

		std::cout << "Loading mesh of " << mesh->mNumVertices << " vertices, "
			<< mesh->mNumFaces << " faces.\n";

		for(int j = 0; j < mesh->mNumVertices; ++j)
		{
			//Vertices
			glm::vec4 vert = glm::vec4(
				mesh->mVertices[j].x,
				mesh->mVertices[j].y,
				mesh->mVertices[j].z,
				1.0);
			v.push_back(vert);
			//Norms
			glm::vec3 norm = glm::vec3(
				mesh->mNormals[j].x,
				mesh->mNormals[j].y,
				mesh->mNormals[j].z);
			n.push_back(norm);
		}

		for(int j = 0; j < mesh->mNumFaces; ++j)
		{
			//Element indices.
			e.push_back((GLushort) mesh->mFaces[j].mIndices[0]);
			e.push_back((GLushort) mesh->mFaces[j].mIndices[1]);
			e.push_back((GLushort) mesh->mFaces[j].mIndices[2]);
		}

		meshes.push_back(new Mesh(v, n, e, _shader));
	}

	return meshes;
}

Mesh::Mesh(const std::vector<glm::vec4>& v, 
		   const std::vector<glm::vec3>& n,
		   const std::vector<GLushort>& e,
		   LightShader* _shader)
	:Solid(_shader)
{
	std::cout << "First vertex x: " << v[0].x << "\n";
	numElems = e.size();

	glGenBuffers(1, &v_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, v_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * v.size(), v.data(), GL_STATIC_DRAW);
	glGenBuffers(1, &n_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, n_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * n.size(), n.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &e_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * e.size(), e.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	v_attrib = shader->getAttribLoc("vPosition");
	n_attrib = shader->getAttribLoc("vNorm");
}

void Mesh::render()
{
	if(!scene) return;
	
	shader->setModelToWorld(modelToWorld);

	shader->use();
	glEnableVertexAttribArray(v_attrib);
	glEnableVertexAttribArray(n_attrib);

	glBindBuffer(GL_ARRAY_BUFFER, v_vbo);
	glVertexAttribPointer(v_attrib, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, n_vbo);
	glVertexAttribPointer(n_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_vbo);

	glDrawElements(GL_TRIANGLES, (GLsizei) numElems, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(v_attrib);
	glDisableVertexAttribArray(n_attrib);
}
