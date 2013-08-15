#include "AOMesh.hpp"

#include "Mesh.hpp"
#include "Intersect.hpp"
#include "Texture.hpp"
#include "SH.hpp"

#include <omp.h>
#include <iostream>
#include <fstream>

#include "SOIL.h"

AOMesh::AOMesh(
	const std::string& bakedFilename,
	LightShader* shader)
	:Renderable(false), shader(shader)
{
	std::vector<AOMeshVertex> mesh;
	std::vector<GLushort> elems;

	readPrebakedFile(mesh, elems, bakedFilename);
	init(mesh, elems);
}

void AOMesh::init(
		const std::vector<AOMeshVertex>& mesh,
		const std::vector<GLushort>& elems)
{
	numElems = elems.size();

	shader->setAmbTexUnit(ambTex->getTexUnit());
	shader->setDiffTexUnit(diffTex->getTexUnit());
	shader->setSpecTexUnit(specTex->getTexUnit());
	shader->setSpecExp(specExp);

	glGenBuffers(1, &v_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, v_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(AOMeshVertex) * mesh.size(),
		mesh.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &e_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * elems.size(),
		elems.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	v_attrib = shader->getAttribLoc("vPosition");
	n_attrib = shader->getAttribLoc("vNorm");
	t_attrib = shader->getAttribLoc("vTexCoord");
}

void AOMesh::render()
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

	glBindVertexBuffer(0, v_vbo, 0, sizeof(AOMeshVertex));
	glVertexAttribFormat(v_attrib, 4, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(v_attrib, 0);
	glVertexAttribFormat(n_attrib, 3, GL_FLOAT, GL_FALSE, offsetof(AOMeshVertex, n));
	glVertexAttribBinding(n_attrib, 0);
	glVertexAttribFormat(t_attrib, 2, GL_FLOAT, GL_FALSE, offsetof(AOMeshVertex, t));
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

void AOMesh::bake(
	const std::string& coarseMeshFilename,
	const std::string& fineMeshFilename,
	const std::string& ambTex,
	const std::string& diffTex,
	const std::string& specTex,
	float specExp,
	int sqrtNSamples)
{
	MeshData coarseData = Mesh::loadSceneFile(coarseMeshFilename);
	MeshData fineData = Mesh::loadSceneFile(fineMeshFilename);
	std::vector<AOMeshVertex> mesh(coarseData.v.size());

	int tid;
	int completedVerts = 0;
	int currPercent = 0;
	int nVerts = static_cast<int>(fineData.v.size());

	std::cout
		<< "> Calculating bent normals..." << std::endl;

	#pragma omp parallel for
	for(int i = 0; i < static_cast<int>(coarseData.v.size()); ++i)
	{
		mesh[i].v = coarseData.v[i];
		mesh[i].t = coarseData.t[i];
		mesh[i].n = glm::vec3(0.0f);

		for(int x = 0; x < sqrtNSamples; ++x)
			for(int y = 0; y < sqrtNSamples; ++y)
			{
				float sqrSize = 1.0f / sqrtNSamples;
				float u = (x * sqrSize);
				float v = (y * sqrSize);
				if(GC::jitterSamples)
				{
					u += randf(0, sqrSize);
					v += randf(0, sqrSize);
				}

				float theta = acos((2 * u) - 1);
				float phi = (2 * PI * v);

				glm::vec3 dir
					(
					sin(theta) * cos(phi),
					sin(theta) * sin(phi),
					cos(theta)
					); 

				/* Continue if dir is not in hemisphere around norm */
				if(glm::dot(dir, coarseData.n[i]) < 0.0f) continue;

				/* Check for intersection with coarse mesh */
				bool intersect = false;

				for(unsigned t = 0; t < coarseData.e.size(); t += 3)
				{
					// Find triangle vertices
					glm::vec3 ta = glm::vec3(coarseData.v[coarseData.e[t]]);
					glm::vec3 tb = glm::vec3(coarseData.v[coarseData.e[t+1]]);
					glm::vec3 tc = glm::vec3(coarseData.v[coarseData.e[t+2]]);

					if(triangleRayIntersect(ta, tb, tc, glm::vec3(fineData.v[i]), dir))
					{
						intersect = true;
						break; // No need to check other triangles.
					}
				}

				if(!intersect)
					mesh[i].n += dir;
			} // end for x, y

		/* Normalize if non-zero (avoid divide by zero!) */
		if(!(abs(mesh[i].n.x) < EPS && 
				abs(mesh[i].n.x) < EPS && 
				abs(mesh[i].n.z) < EPS))
				mesh[i].n = glm::normalize(mesh[i].n);
	}

	std::cout 
		<< "> Calculating occlusion (may take some time) ..." << std::endl;

	std::vector<float> fineOccl(fineData.v.size());
	std::vector<glm::vec2> fineTex(fineData.t.size());

	#pragma omp parallel private(tid)
	{
		tid = omp_get_thread_num();

		//Calculate per vert occl and bent norms
		#pragma omp for
		for(int i = 0; i < static_cast<int>(fineData.v.size()); ++i)
		{
			fineOccl[i] = 0.0f;
			fineTex[i] = fineData.t[i];

			float sqrSize = 1.0f / sqrtNSamples;

			/* Perform stratified sampling in the hemisphere around the norm */
			/* Sample over whole sphere first */
			for(int x = 0; x < sqrtNSamples; ++x)
				for(int y = 0; y < sqrtNSamples; ++y)
				{
					float sqrSize = 1.0f / sqrtNSamples;
					float u = (x * sqrSize);
					float v = (y * sqrSize);
					if(GC::jitterSamples)
					{
						u += randf(0, sqrSize);
						v += randf(0, sqrSize);
					}

					float theta = acos((2 * u) - 1);
					float phi = (2 * PI * v);

					glm::vec3 dir
						(
						sin(theta) * cos(phi),
						sin(theta) * sin(phi),
						cos(theta)
						); 

					/* Continue if dir is not in hemisphere around norm */
					if(glm::dot(dir, fineData.n[i]) < 0.0f) continue;

					/* Check for intersection with coarse mesh */
					bool intersect = false;

					for(unsigned t = 0; t < coarseData.e.size(); t += 3)
					{
						// Find triangle vertices
						glm::vec3 ta = glm::vec3(coarseData.v[coarseData.e[t]]);
						glm::vec3 tb = glm::vec3(coarseData.v[coarseData.e[t+1]]);
						glm::vec3 tc = glm::vec3(coarseData.v[coarseData.e[t+2]]);

						if(triangleRayIntersect(ta, tb, tc, glm::vec3(fineData.v[i]), dir))
						{
							intersect = true;
							break; // No need to check other triangles.
						}
					}

					if(!intersect)
					{
						fineOccl[i] += 1.0f;
					}
				} // end for x, y

			fineOccl[i] /= (0.5f * sqrtNSamples * sqrtNSamples);

			completedVerts++;
			if(tid == 0)
			{
				int percent = (completedVerts * 100) / nVerts;
				if(percent > currPercent)
				{
					currPercent = percent;
					std::cout << "*";
					if(percent % 10 == 0 && percent != 100) 
						std::cout << " " << percent << "% complete" << std::endl; 
				}
			}
		} // end parallel for
	} // end parallel
	std::cout << " 100% complete" << std::endl;

	AOMesh::renderOcclToImage(fineOccl, ambTex, coarseMeshFilename + ".aoamb.bmp", fineData);

	writePrebakedFile(mesh, coarseData.e,
		coarseMeshFilename + ".aoamb.bmp", diffTex, specTex, specExp,
		coarseMeshFilename + ".ao");
}


void AOMesh::writePrebakedFile(
		const std::vector<AOMeshVertex>& mesh,
		const std::vector<GLushort>& elems,
		const std::string& ambTex,
		const std::string& diffTex,
		const std::string& specTex,
		float specExp,
	 	const std::string& filename)
{
	std::ofstream file(filename);

	file << "Vertices" << std::endl;

	for(auto v = mesh.begin(); v != mesh.end(); ++v)
	{
		file
			<< v->v.x << " "
			<< v->v.y << " "
			<< v->v.z << " " 
			<< v->v.w << std::endl;
		file
			<< v->n.x << " "
			<< v->n.y << " "
			<< v->n.z << std::endl;
		file 
			<< v->t.x << " "
			<< v->t.y << std::endl;
	}

	file << "Elements" << std::endl;

	for(auto e = elems.begin(); e != elems.end(); ++e)
		file << *e << std::endl;

	file << "Textures" << std::endl;

	file << ambTex  << std::endl;
	file << diffTex << std::endl;
	file << specTex << std::endl;
	file << specExp << std::endl;

	file.close();
}

void AOMesh::readPrebakedFile(
	std::vector<AOMeshVertex>& mesh,
	std::vector<GLushort>& elems,
 	const std::string& filename)
{
	std::ifstream file(filename);

	if(!file) throw(new MeshFileException);

	char ignore[10];

	file.getline(ignore, 10); //Throw the "Vertices" line.

	float next;

	while(file >> next)
	{
		AOMeshVertex vert;

		vert.v.x = next;
		file >> vert.v.y;
		file >> vert.v.z;
		file >> vert.v.w;

		file >> vert.n.x;
		file >> vert.n.y;
		file >> vert.n.z;

		file >> vert.t.x;
		file >> vert.t.y;

		mesh.push_back(vert);
	}

	file.clear();
	file.getline(ignore, 10); //Throw the "Elements" line.

	int elem;
	while(file >> elem)
		elems.push_back(static_cast<GLushort>(elem));

	file.clear();
	file.getline(ignore, 10); //Throw the "Textures" line.

	char readFilename[40];

	file.getline(readFilename, 40);
	ambTex = new Texture(std::string(readFilename));
	file.getline(readFilename, 40);
	diffTex = new Texture(std::string(readFilename));
	file.getline(readFilename, 40);
	specTex = new Texture(std::string(readFilename));

	file >> specExp;

	numElems = elems.size();

	file.close();
}

void AOMesh::renderOcclToImage(
	const std::vector<float>& vertOccl,
	const std::string& ambIm,
	const std::string& bakedIm,
	const MeshData& data)
{
	float avgOccl = 0.0f;
	for(auto o = vertOccl.begin(); o != vertOccl.end(); ++o)
		avgOccl += *o;
	avgOccl /= static_cast<float>(vertOccl.size());

	/* Most image formats are upside down, so load data and flip it. */
	int width, height, channels;
	unsigned char* ambDataFlip = SOIL_load_image(
		ambIm.c_str(),
		&width, &height, &channels,
		SOIL_LOAD_RGB);

	unsigned char* ambData = static_cast<unsigned char*>(
		malloc(width*height*channels*sizeof(unsigned char)));

	for(int u = 0; u < width; ++u)
		for(int v = 0; v < height; ++v)
			for(int c = 0; c < 3; ++c)
				ambData[(u + v*width)*3 + c] =
					ambDataFlip[(u + ((height-v)-1)*width)*3 + c];

	SOIL_free_image_data(ambDataFlip);

	// Create framebuffer
	GLuint frame;
	glGenFramebuffers(1, &frame);
	glBindFramebuffer(GL_FRAMEBUFFER, frame);

	// Create renderbuffer and attach to framebuffer
	GLuint render;
	glGenRenderbuffers(1, &render);
	glBindRenderbuffer(GL_RENDERBUFFER, render);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, render);

	glViewport(0, 0, width, height);

	// Create shader object
	Shader occlShader(false, "AOBake", false, false);
	GLuint tex_attrib = occlShader.getAttribLoc("vTexCoord");
	GLuint occl_attrib = occlShader.getAttribLoc("vOccl");

	// Pass data to GPU
	GLuint tex_vbo;
	glGenBuffers(1, &tex_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, tex_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * data.t.size(), data.t.data(), GL_STATIC_DRAW);
	GLuint occl_vbo;
	glGenBuffers(1, &occl_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, occl_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertOccl.size(), vertOccl.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint elem_ebo;
	glGenBuffers(1, &elem_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elem_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * data.e.size(), data.e.data(), GL_STATIC_DRAW);
	
	// Rendering setup
	// Store current state
	GLfloat clearCol[4];
	GLboolean faceCull = GL_TRUE;
	glGetFloatv(GL_COLOR_CLEAR_VALUE, clearCol);
	glGetBooleanv(GL_CULL_FACE, &faceCull);
	// Modify state
	glClearColor(avgOccl, avgOccl, avgOccl, 1.0f);
	glDisable(GL_CULL_FACE);
	glClear(GL_COLOR_BUFFER_BIT);

	occlShader.use();
	glEnableVertexAttribArray(tex_attrib);
	glEnableVertexAttribArray(occl_attrib);

	glBindVertexBuffer(0, tex_vbo, 0, sizeof(glm::vec2));
	glVertexAttribFormat(tex_attrib, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(tex_attrib, 0);
	glBindVertexBuffer(1, occl_vbo, 0, sizeof(float));
	glVertexAttribFormat(occl_attrib, 1, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(occl_attrib, 1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Render
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(data.e.size()), GL_UNSIGNED_SHORT, 0);

	// Rendering cleanup
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(tex_attrib);
	glDisableVertexAttribArray(occl_attrib);
	glDeleteBuffers(1, &tex_vbo);
	glDeleteBuffers(1, &occl_vbo);
	glDeleteBuffers(1, &elem_ebo);
	glUseProgram(0);
	//Restore old state
	if(faceCull == GL_TRUE) glEnable(GL_CULL_FACE);
	else glDisable(GL_CULL_FACE);
	glClearColor(clearCol[0], clearCol[1], clearCol[2], clearCol[3]);

	// Pull rendered image from GPU
	std::vector<float> renderedData(width*height);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, width, height, GL_RED, GL_FLOAT, renderedData.data());

	std::vector<unsigned char> bakedImage(width * height * 3);

	for(int u = 0; u < width; ++u)
		for(int v = 0; v < height; ++v)
			for(int c = 0; c < 3; ++c)
				bakedImage[(u + v*width)*3 + c] = static_cast<unsigned char>(
					renderedData[u + v*width] * 
					(static_cast<float>(ambData[(u + v*width)* channels + c]) / 255.0f)
					* 255.0f);

	SOIL_save_image
		(
			bakedIm.c_str(),
			SOIL_SAVE_TYPE_BMP,
			width, height, 3,
			bakedImage.data()
		);

	// More cleanup
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &frame);
	glDeleteRenderbuffers(1, &render); 

	free(ambData);
}
