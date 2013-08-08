#include "Mesh.hpp"

bool fileExists(const std::string& filename)
{
	std::ifstream file(filename);
	return file ? true : false;
}

MeshData Mesh::loadSceneFile(
	const std::string& filename,
	TexCoordGenMode mode)
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
			//Vertices
			glm::vec4 vert(
				mesh->mVertices[j].x,
				mesh->mVertices[j].y,
				mesh->mVertices[j].z,
				1.0);
			d.v.push_back(vert);
			//Norms
			glm::vec3 norm(
				mesh->mNormals[j].x,
				mesh->mNormals[j].y,
				mesh->mNormals[j].z);
			d.n.push_back(norm);
		}

		if(mode != DONOTGEN)
			genTexCoords(d, mode);

		else if(mesh->HasTextureCoords(0))
		{
			for(int j = 0; j < (int) mesh->mNumVertices; ++j)
			{
				glm::vec2 tex(
					mesh->mTextureCoords[0][j].x,
					mesh->mTextureCoords[0][j].y);
				d.t.push_back(tex);
			}
		}

		else
			genTexCoords(d, CYLINDRICAL);

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

MeshData Mesh::combineData(const std::vector<MeshData>& data)
{
	MeshData comb;

	for(auto d = data.begin(); d != data.end(); ++d)
	{
		GLushort elemBase = static_cast<GLushort>(comb.e.size());

		for(auto v = d->v.begin(); v != d->v.end(); ++v)
			comb.v.push_back(*v);
		for(auto n = d->n.begin(); n != d->n.end(); ++n)
			comb.n.push_back(*n);
		for(auto e = d->e.begin(); e != d->e.end(); ++e)
			comb.e.push_back(elemBase + *e);
		for(auto t = d->t.begin(); t != d->t.end(); ++t)
			comb.t.push_back(*t);
	}

	return comb;
}

void Mesh::genTexCoords(MeshData& data, TexCoordGenMode mode)
{
	switch(mode)
	{
	case CYLINDRICAL:
		genTexCoordsCylindrical(data);
		break;
	}
}

Mesh::Mesh(		
	const std::string& meshFilename,
	Texture* ambTex,
	Texture* diffTex,
	Texture* specTex,
	float exponent,
	LightShader* shader,
	TexCoordGenMode mode)
	:Renderable(false), shader(shader),
	ambTex(ambTex), diffTex(diffTex),
	specTex(specTex), specExp(exponent)
{
	MeshData data = loadSceneFile(meshFilename, mode);
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
		vert.t = data.t[i];
		vertBuffer.push_back(vert);
	}

	shader->setAmbTexUnit(ambTex->getTexUnit());
	shader->setDiffTexUnit(diffTex->getTexUnit());
	shader->setSpecTexUnit(specTex->getTexUnit());
	shader->setSpecExp(specExp);

	glGenBuffers(1, &v_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, v_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVertex) * vertBuffer.size(),
		vertBuffer.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &e_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * data.e.size(),
		data.e.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	v_attrib = shader->getAttribLoc("vPosition");
	n_attrib = shader->getAttribLoc("vNorm");
	t_attrib = shader->getAttribLoc("vTexCoord");
}

void Mesh::render()
{
	if(!scene) return;
	
	shader->setModelToWorld(modelToWorld);

	shader->setAmbTexUnit(ambTex->getTexUnit());
	shader->setDiffTexUnit(diffTex->getTexUnit());
	shader->setSpecTexUnit(specTex->getTexUnit());
	shader->setSpecExp(specExp);

	shader->use();
	glEnableVertexAttribArray(v_attrib);
	glEnableVertexAttribArray(n_attrib);
	glEnableVertexAttribArray(t_attrib);

	glBindVertexBuffer(0, v_vbo, 0, sizeof(MeshVertex));
	glVertexAttribFormat(v_attrib, 4, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(v_attrib, 0);
	glVertexAttribFormat(n_attrib, 3, GL_FLOAT, GL_FALSE, offsetof(MeshVertex, n));
	glVertexAttribBinding(n_attrib, 0);
	glVertexAttribFormat(t_attrib, 2, GL_FLOAT, GL_FALSE, offsetof(MeshVertex, t));
	glVertexAttribBinding(t_attrib, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_vbo);

	glDrawElements(GL_TRIANGLES, (GLsizei) numElems, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(v_attrib);
	glDisableVertexAttribArray(n_attrib);
	glDisableVertexAttribArray(t_attrib);

	glUseProgram(0);
}

void Mesh::genTexCoordsCylindrical(MeshData& data)
{
	float highest = FLT_MIN;
	float lowest =  FLT_MAX;
	glm::vec2 center(0.0f);

	for(auto v = data.v.begin(); v != data.v.end(); ++v)
	{
		if(highest < v->y) highest = v->y;
		if(lowest  > v->y) lowest  = v->y;
		center += glm::vec2(v->x, v->z);
	}

	center /= data.v.size();

	float height = highest - lowest;

	for(auto v = data.v.begin(); v != data.v.end(); ++v)
	{
		glm::vec2 texCoord;

		texCoord.y = (v->y - lowest) / height;
		texCoord.x = (atan2(v->z - center.y, v->x - center.x)  + PI) / (2.0f * PI);

		data.t.push_back(texCoord);
	}
}
