#include "Mesh.hpp"

#include "Scene.hpp"
#include "Texture.hpp"
#include "Intersect.hpp"

#include <float.h>
#include <omp.h>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include <iostream>
#include <fstream>

bool fileExists(const std::string& filename)
{
	std::ifstream file(filename);
	return file ? true : false;
}

MeshData Mesh::loadSceneFile(
	const std::string& filename)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filename,
		aiProcess_CalcTangentSpace | aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices | aiProcess_SortByPType |
		aiProcess_GenSmoothNormals);

	if(!scene) throw MeshFileException(
		"Mesh file " + filename + " could not be loaded\n");

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

		if(mesh->HasTextureCoords(0))
		{
			for(int j = 0; j < (int) mesh->mNumVertices; ++j)
			{
				glm::vec2 tex(
					mesh->mTextureCoords[0][j].x,
					mesh->mTextureCoords[0][j].y);
				d.t.push_back(tex);
			}
		}

		else for(int j = 0; j < (int) mesh->mNumVertices; ++j)
			d.t.push_back(glm::vec2(0.0f));

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

	std::cout << "All meshes loaded from " + filename + ".\n";

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

Mesh::Mesh(		
	const std::string& meshFilename,
	Texture* ambTex,
	Texture* diffTex,
	Texture* specTex,
	float exponent,
	LightShader* shader)
	:Renderable(false), shader(shader),
	ambTex(ambTex), diffTex(diffTex),
	specTex(specTex), specExp(exponent)
{
	MeshData data;
	try
	{
		 data = loadSceneFile(meshFilename);
	} 
	catch(const MeshFileException& e)
	{
		std::cout << e.msg;
		return;
	}
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

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, v_vbo);
	glEnableVertexAttribArray(v_attrib);
	glEnableVertexAttribArray(n_attrib);
	glEnableVertexAttribArray(t_attrib);
	glVertexAttribPointer(v_attrib, 4, GL_FLOAT, GL_FALSE, sizeof(MeshVertex),
		reinterpret_cast<GLvoid*>(offsetof(MeshVertex, v)));
	glVertexAttribPointer(n_attrib, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), 
		reinterpret_cast<GLvoid*>(offsetof(MeshVertex, n)));
	glVertexAttribPointer(t_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), 
		reinterpret_cast<GLvoid*>(offsetof(MeshVertex, t)));

	glBindVertexArray(0);
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

	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_vbo);

	glDrawElements(GL_TRIANGLES, (GLsizei) numElems, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glUseProgram(0);
}
