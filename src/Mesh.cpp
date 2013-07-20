#include "Mesh.hpp"
namespace
{
	std::vector<MeshData> loadFileData(const std::string& filename)
	{
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(filename,
			aiProcess_CalcTangentSpace | aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices | aiProcess_SortByPType );

		if(!scene) throw new MeshFileException;

		std::cout << "Loading from file: " << filename << "\n" 
			<< scene->mNumMeshes << " meshes found.\n";

		std::vector<MeshData> data;

		for(int i = 0; i < (int) scene->mNumMeshes; ++i)
		{
			aiMesh* mesh = scene->mMeshes[i];

			MeshData d;

			std::cout << "Loading mesh of " << mesh->mNumVertices << " vertices, "
				<< mesh->mNumFaces << " faces.\n";

			for(int j = 0; j < (int) mesh->mNumVertices; ++j)
			{
				//Vertices
				glm::vec4 vert = glm::vec4(
					mesh->mVertices[j].x,
					mesh->mVertices[j].y,
					mesh->mVertices[j].z,
					1.0);
				d.v.push_back(vert);
				//Norms
				glm::vec3 norm = glm::vec3(
					mesh->mNormals[j].x,
					mesh->mNormals[j].y,
					mesh->mNormals[j].z);
				d.n.push_back(norm);
			}

			for(int j = 0; j < (int) mesh->mNumFaces; ++j)
			{
				//Element indices.
				d.e.push_back((GLushort) mesh->mFaces[j].mIndices[0]);
				d.e.push_back((GLushort) mesh->mFaces[j].mIndices[1]);
				d.e.push_back((GLushort) mesh->mFaces[j].mIndices[2]);
			}

			data.push_back(d);
		}
		return data;
	}
}

std::vector<Mesh*> Mesh::loadFile(const std::string& filename,
	LightShader* _shader)
{
	std::vector<Mesh*> meshes;
	std::vector<MeshData> data = loadFileData(filename);

	for(std::vector<MeshData>::iterator i = data.begin(); i != data.end(); ++i)
		meshes.push_back(new Mesh(*i, _shader));

	return meshes;
}

Mesh::Mesh(const MeshData& d, LightShader* _shader)
	:Solid(_shader)
{
	numElems = d.e.size();

	glGenBuffers(1, &v_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, v_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * d.v.size(), d.v.data(), GL_STATIC_DRAW);
	glGenBuffers(1, &n_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, n_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * d.n.size(), d.n.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &e_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * d.e.size(), d.e.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	v_attrib = shader->getAttribLoc("vPosition");
	n_attrib = shader->getAttribLoc("vNorm");
}

void Mesh::render()
{
	if(!scene) return;
	
	shader->setModelToWorld(modelToWorld);
	shader->setMaterial(material);

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

std::vector<DiffPRTMesh*> DiffPRTMesh::loadFile(
	const std::string& filename,
	int nBands,
	SHShader* _shader)
{
	std::vector<DiffPRTMesh*> meshes;
	std::vector<MeshData> data = loadFileData(filename);

	for(std::vector<MeshData>::iterator i = data.begin(); i != data.end(); ++i)
		meshes.push_back(new DiffPRTMesh(*i, nBands, _shader));

	return meshes;
}


DiffPRTMesh::DiffPRTMesh(const MeshData& d, int _nBands, SHShader* _shader)
	:Solid(_shader), nBands(_nBands)
{
	nCoeffts = (nBands + 1)*(nBands + 1);

	numElems = d.e.size();

	std::vector<glm::vec3> s;

	for(std::vector<glm::vec3>::const_iterator i = d.n.begin(); i != d.n.end(); ++i)
	{
		std::vector<glm::vec3> coeffts = SH::shProject(Scene::sqrtSHSamples, 3, 
			[&i](double theta, double phi) -> glm::vec3 
				{
					glm::vec3 dir
						(
						sin(theta) * cos(phi),
						sin(theta) * sin(phi),
						cos(phi)
						);
					double proj = glm::dot(dir, *i);
					proj = (proj > 0.0 ? proj : 0.0);
					return glm::vec3(proj, proj, proj);
				}
			);

		s.insert(s.end(), coeffts.begin(), coeffts.end());
	}

	glGenBuffers(1, &v_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, v_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * d.v.size(), d.v.data(), GL_STATIC_DRAW);
	glGenBuffers(1, &s_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * s.size(), s.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &e_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * d.e.size(), d.e.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	v_attrib = shader->getAttribLoc("vPos");
	s_attrib = shader->getAttribLoc("transferCoeffts");
}

void DiffPRTMesh::render()
{
	if(!scene) return;
	
	shader->setModelToWorld(modelToWorld);

	shader->use();
	glEnableVertexAttribArray(v_attrib);
	glEnableVertexAttribArray(s_attrib);

	glBindBuffer(GL_ARRAY_BUFFER, v_vbo);
	glVertexAttribPointer(v_attrib, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
	for(int c = 0; c < Scene::nSHCoeffts; ++c)
		glVertexAttribPointer(s_attrib + c, 3, GL_FLOAT, GL_FALSE,
			3 * sizeof(float) * Scene::nSHCoeffts,
			reinterpret_cast<GLvoid*> (3 * sizeof(float) * c));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_vbo);

	glDrawElements(GL_TRIANGLES, (GLsizei) numElems, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(v_attrib);
	glDisableVertexAttribArray(s_attrib);
}