#include "Mesh.hpp"

bool fileExists(const std::string& filename)
{
	std::ifstream file(filename);
	return file ? true : false
}

MeshData Mesh::loadSceneFile(
	const std::string& filename, Material mat)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filename,
		aiProcess_CalcTangentSpace | aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices | aiProcess_SortByPType |
		aiProcess_GenSmoothNormals);

	if(!scene) throw new MeshFileException;

	std::cout << "Loading scene from file: " << filename << std::endl;
	std::cout << "> " << scene->mNumMeshes << " meshes in scene." << std::endl;

	std::vector<MeshData> data;

	for(int i = 0; i < (int) scene->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[i];

		MeshData d;

		std::cout << "> Mesh " << i << " of "
			<< mesh->mNumVertices << " vertices, "
			<< mesh->mNumFaces << " triangles.\n";

		for(int j = 0; j < (int) mesh->mNumVertices; ++j)
		{
			d.M.push_back(mat);

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
			//Material indices (all 0)
			d.m.push_back(0);
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

	MeshData mesh = combineData(data);

	return mesh;
}

MeshData Mesh::loadSceneFiles(
	const std::vector<std::string>& filenames, 
	const std::vector<Material>& mats)
{
	std::vector<MeshData> dataVec;

	for(unsigned i = 0; i < filename.size(); ++i)
	{
		MeshData data = loadSceneFile(filename[i], mat[i]);
		dataVec.push_back(data);
	}

	return combineData(dataVec);
}

MeshData Mesh::combineData(const std::vector<MeshData>& data)
{
	MeshData comb;

	for(auto d = data.begin(); d != data.end(); ++d)
	{
		GLushort elemBase = static_cast<GLushort>(comb.e.size());
		GLushort matBase  = static_cast<GLushort>(comb.M.size());

		for(auto v = d->v.begin(); v != d->v.end(); ++v)
			comb.v.push_back(*v);
		for(auto n = d->n.begin(); n != d->n.end(); ++n)
			comb.n.push_back(*n);
		for(auto e = d->e.begin(); e != d->e.end(); ++e)
			comb.e.push_back(elemBase + *e);
		for(auto m = d->m.begin(); m != d->m.end(); ++m)
			comb.m.push_back(matBase + *m);
		for(auto M = d->M.begin(); M != d->M.end(); ++M)
			comb.M.push_back(*M);
	}

	return comb;
}

Mesh::Mesh(const std::string& filename, 
	const Material& mat,
	LightShader* shader)
	:shader(shader)
{
	MeshData data = loadSceneFile(filename, mat);
	init(data);
}

Mesh(const std::vector<std::string>& filenames, 
	const std::vector<Material>& mats,
	LightShader* shader)
	:shader(shader)
{
	MeshData data = loadSceneFiles(filenames, mats);
	init(data);
}

void Mesh::init(const MeshData& data)
{
	numElems = data.e.size();

	std::vector<MeshVertex> vertBuffer;
	for(unsigned i = 0; i < data.v.size(); ++i)
	{
		MeshVertex vert;
		vert.v = data.v[i];
		vert.n = data.n[i];
		vert.m = data.m[i];
	}

	glGenBuffers(1, &v_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, v_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVertex) * vertexBuffer.size(),
		vertexBuffer.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &e_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * d.e.size(),
		d.e.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	v_attrib = shader->getAttribLoc("vPosition");
	n_attrib = shader->getAttribLoc("vNorm");
	m_attrib = shader->getAttribLoc("vMatIndex");
}

void Mesh::render()
{
	if(!scene) return;
	
	shader->setModelToWorld(modelToWorld);
	static_cast<LightShader*>(shader)->setMaterials(mats);

	shader->use();
	glEnableVertexAttribArray(v_attrib);
	glEnableVertexAttribArray(n_attrib);
	glEnableVertexAttribArray(m_attrib);

	glBindVertexBuffer(0, v_vbo, 0, sizeof(MeshVertex));
	glVertexAttribFormat(v_attrib, 4, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(v_attrib, 0);
	glVertexAttribFormat(n_attrib, 3, GL_FLOAT, GL_FALSE, offsetof(MeshVertex, n));
	glVertexAttribBinding(n_attrib, 0);
	glVertexAttribFormat(m_attrib, 1, GL_INT, GL_FALSE, offsetof(MeshVertex, m));
	glVertexAttribBinding(m_attrib, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_vbo);

	glDrawElements(GL_TRIANGLES, (GLsizei) numElems, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(v_attrib);
	glDisableVertexAttribArray(n_attrib);
	glDisableVertexAttribArray(m_attrib);

	glUseProgram(0);
}
