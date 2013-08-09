#include "PRTMesh.hpp"

PRTMesh::PRTMesh(
	const std::string& bakedFilename,
	SHShader* shader)
	:Renderable(false), shader(shader)
{
	std::vector<PRTMeshVertex> mesh;
	std::vector<GLushort> elems;
	std::vector<std::string> coefftFilenames;

	readPrebakedFile(mesh, elems, coefftFilenames, bakedFilename);
	
	arrTex = new ArrayTexture(coefftFilenames);

	init(mesh, elems);
}

PRTMesh::~PRTMesh()
{
	delete arrTex;
}

void PRTMesh::bake(
	PRTMode mode,
	const std::string& meshFilename,
	const std::string& diffTex,
	int sqrtNSamples,
	int nBands,
	int nBounces,
	TexCoordGenMode texMode)
{
	MeshData data = Mesh::loadSceneFile(meshFilename, texMode);
	std::vector<PRTMeshVertex> mesh(data.v.size());
	std::vector<std::vector<glm::vec3>> transfer;

	int width, height, channels;
	unsigned char* diffData = SOIL_load_image(
		diffTex.c_str(),
		&width, &height, &channels,
		SOIL_LOAD_AUTO);

	int tid;
	int completedVerts = 0;
	int currPercent = 0;
	int nVerts = static_cast<int>(data.v.size());

	std::cout 
		<< "Calculating transfer coeffts (may take some time) ..." << std::endl;

	//#pragma omp parallel private(tid)
	{
		//tid = omp_get_thread_num();
		tid = 0;
		//#pragma omp for
		for(int i = 0; i < nVerts; ++i)
		{
			mesh[i].v = data.v[i];
			mesh[i].t = data.t[i];

			std::vector<glm::vec3> coeffts;

			if(mode == UNSHADOWED)
				coeffts = SH::shProject(sqrtNSamples, nBands, 
					[&data, &i, &diffData, &width, &height, &channels]
					(float theta, float phi) -> glm::vec3 
						{
							glm::vec3 dir
								(
								sin(theta) * cos(phi),
								sin(theta) * sin(phi),
								cos(theta)
								);
							dir = glm::normalize(dir);
							glm::vec3 norm = glm::normalize(data.n[i]);

							glm::vec3 surfColor = texLookup(
								diffData, data.t[i],
								width, height, channels);

							float proj = glm::dot(dir, norm);
							proj = (proj > 0.0f ? proj : 0.0f);

							return proj * surfColor;
						}
					);

			else // mode == SHADOWED || mode == INTERREFLECTED
				coeffts = SH::shProject(sqrtNSamples, nBands, 
					[&data, &i, &diffData, &width, &height, &channels]
					(float theta, float phi) -> glm::vec3 
						{
							bool intersect = false;

							glm::vec3 dir
								(
								sin(theta) * cos(phi),
								sin(theta) * sin(phi),
								cos(theta)
								);
							dir = glm::normalize(dir);
							glm::vec3 norm = glm::normalize(data.n[i]);

							float proj = glm::dot(dir, norm);

							if(proj <= 0.0f) return glm::vec3(0.0, 0.0, 0.0);

							// For each triangle in mesh
							for(size_t e = 0; e < data.e.size(); e += 3)
							{
								// Find triangle vertices
								glm::vec3 ta = glm::vec3(data.v[data.e[e]]);
								glm::vec3 tb = glm::vec3(data.v[data.e[e+1]]);
								glm::vec3 tc = glm::vec3(data.v[data.e[e+2]]);

								// Check for intersection
								if(triangleRayIntersect(
									ta, tb, tc, glm::vec3(data.v[i]), dir))
								{
									intersect = true;
									break;
								}
							}
							// Light is blocked, return 0.
							if(intersect) return glm::vec3(0.0, 0.0, 0.0);
							// Light not occluded.

							glm::vec3 surfColor = texLookup(
								diffData, data.t[i],
								width, height, channels);

							return proj * surfColor;
						}
					);

			transfer.push_back(coeffts);

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

	if(mode == INTERREFLECTED)
	{
		std::cout << "Interreflection pass begins...\n";
		PRTMesh::interreflect(data, diffTex, nBands, sqrtNSamples, nBounces, transfer);
	}

	free(diffData);

	std::vector<std::string> coefftFilenames;
	std::string prebakedFilename = PRTMesh::genPrebakedFilename(meshFilename, mode, nBands);

	PRTMesh::writeTransferToTextures(transfer, 
		data, prebakedFilename, coefftFilenames, width, height);

	PRTMesh::writePrebakedFile(mesh, data.e, coefftFilenames, prebakedFilename);
}

std::string PRTMesh::genPrebakedFilename(
	const std::string& filename,
	PRTMode mode,
	int nBands)
{
	std::string prebakedFilename = filename + ".prt";

	switch(mode)
	{
	case UNSHADOWED:
		prebakedFilename += "u";
		break;
	case SHADOWED:
		prebakedFilename += "s";
		break;
	case INTERREFLECTED:
		prebakedFilename += "i";
		break;
	}

	prebakedFilename += std::to_string(static_cast<long long>(nBands));

	return prebakedFilename;
}

void PRTMesh::writePrebakedFile(
	const std::vector<PRTMeshVertex>& mesh,
	const std::vector<GLushort>& elems,
	const std::vector<std::string>& coefftTex,
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
			<< v->t.x << " "
			<< v->t.y << std::endl;
	}

	file << "Elements" << std::endl;

	for(auto e = elems.begin(); e != elems.end(); ++e)
		file << *e << std::endl;

	file << "Coefft Textures" << std::endl;

	for(auto t = coefftTex.begin(); t != coefftTex.end(); ++t)
		file << *t << std::endl;

	file.close();
}

void PRTMesh::readPrebakedFile(
	std::vector<PRTMeshVertex>& mesh,
	std::vector<GLushort>& elems,
	std::vector<std::string>& coefftFilenames,
 	const std::string& filename)
{
	std::ifstream file(filename);

	if(!file) throw(new MeshFileException);

	char ignore[30];

	file.getline(ignore, 30); //Throw the "Vertices" line.

	float next;

	while(file >> next)
	{
		PRTMeshVertex vert;

		vert.v.x = next;
		file >> vert.v.y;
		file >> vert.v.z;
		file >> vert.v.w;

		file >> vert.t.x;
		file >> vert.t.y;

		mesh.push_back(vert);
	}

	file.clear();
	file.getline(ignore, 30); //Throw the "Elements" line.

	int elem;
	while(file >> elem)
		elems.push_back(static_cast<GLushort>(elem));

	file.clear();
	file.getline(ignore, 30); //Throw the "Coefft Textures" line.

	char coefftFilename[40];
	while(file.getline(coefftFilename, 40))
		coefftFilenames.push_back(std::string(coefftFilename));

	numElems = elems.size();

	file.close();
}

void PRTMesh::interreflect(
	const MeshData& data, const std::string& diffTex,
	int nBands, int sqrtNSamples, int nBounces,
	std::vector<std::vector<glm::vec3>>& transfer)
{
	int width, height, channels;
	unsigned char* diffData = SOIL_load_image(
		diffTex.c_str(),
		&width, &height, &channels,
		SOIL_LOAD_AUTO);

	std::vector<std::vector<glm::vec3>> prevBounce(transfer);
	std::vector<std::vector<glm::vec3>> currBounce(transfer.size());

	float sqrSize = 1.0f / sqrtNSamples;

	for(int b = 0; b < nBounces; ++b)
	{
		std::cout << "Calculating bounce " << b + 1
			<< " of " << nBounces << std::endl;

		int tid;
		int completedVerts = 0;
		int currPercent = 0;
		int nVerts = static_cast<int>(data.v.size());

		#pragma omp parallel private(tid, currBounce, prevBounce)
		{
			tid = omp_get_thread_num();
			#pragma omp for
			for(int i = 0; i < nVerts; ++i)
			{
				/* Zero currBounce */
				for(auto v = currBounce.begin(); v != currBounce.end(); ++v)
				{
					std::vector<glm::vec3> vert;
					for(int i = 0; i < nBands*nBands; ++i)
						vert.push_back(glm::vec3(0.0f));
					*v = vert;
				}

				for(int x = 0; x < sqrtNSamples; ++x)
					for(int y = 0; y < sqrtNSamples; ++y)
					{
						float sqrWidth = 1 / (float) sqrtNSamples;

						float u = (x * sqrSize);
						float v = (y * sqrSize);
						if(GC::jitterSamples)
						{
							u += randf(0, sqrWidth);
							v += randf(0, sqrWidth);
						}

						float theta = acos((2 * u) - 1);
						float phi = (2 * PI * v);

						glm::vec3 dir
							(
							sin(theta) * cos(phi),
							sin(theta) * sin(phi),
							cos(theta)
							);

						if(glm::dot(data.n[i], dir) <= 0.0f) // dir not in hemisphere
							continue;

						/* Find closest intersection */
						glm::vec3 closest(-1.0, -1.0, -1.0);
						int closestTri = -1;
						for(int t = 0; t < static_cast<int>(data.e.size()); t+=3)
						{
							glm::vec3 ta = glm::vec3(data.v[data.e[t]]);
							glm::vec3 tb = glm::vec3(data.v[data.e[t+1]]);
							glm::vec3 tc = glm::vec3(data.v[data.e[t+2]]);

							glm::vec3 intersect = getTriangleRayIntersection(
								ta, tb, tc, glm::vec3(data.v[i]), dir);

							if(intersect.z > 0.0f &&
								(intersect.z < closest.z || closest.z < 0.0f))
							{
								closest = intersect;
								closestTri = t;
							}
						}

						if(closestTri == -1) // No intersections
							continue;

						// Add contribution using coeffts interpolated over triangle.
						float tu = closest.x;
						float tv = closest.y;

						glm::vec2 intersectTexPos = 
							(1-(tu+tv)) * data.t[closestTri  ] +
							tu          * data.t[closestTri+1] +
							tv          * data.t[closestTri+2];

						glm::vec3 intersectColor = texLookup(
							diffData, intersectTexPos, width, height, channels);

						for(int c = 0; c < nBands*nBands; ++c)
						{
							glm::vec3 avgPrevBounce = 
								((1-(tu+tv)) * prevBounce[data.e[closestTri]][c] +
								tu * prevBounce[data.e[closestTri + 1]][c] +
								tv * prevBounce[data.e[closestTri + 2]][c]);

							currBounce[i][c] += 
								glm::dot(data.n[i], dir) * intersectColor * avgPrevBounce;
						}
					}

				// Normalize coeffts.
				for(int c = 0; c < nBands*nBands; ++c)
					currBounce[i][c] *= 2.0 / (PI * sqrtNSamples);	

				// Add to vertBuffer coeffts.
				for(int c = 0; c < nBands*nBands; ++c)
					transfer[i][c] += currBounce[i][c];

				// Copy currentBounce into previousBounce ready for next iteration.
				prevBounce[i] = currBounce[i];

				completedVerts++;
				if(tid == 0)
				{
					int percent = (completedVerts * 100) / nVerts;
					if(percent > currPercent)
					{
						currPercent = percent;
						std::cout << "*";
						if(percent % 10 == 0 && percent != 100) 
							std::cout << " " << percent 
								<< "% complete" << std::endl; 
					}
				}
			} // end parallel for
		} // end parallel
		std::cout << " 100% complete" << std::endl;
	} // end bounces

	free(diffData);
}

void PRTMesh::renderCoefftToTexture(
	const std::vector<glm::vec3>& coefft,
	const std::string& texFilename,
	const MeshData& data,
	int width, int height)
{
	glm::vec3 avgCoefft  = glm::vec3(0.0f);
	for(auto c = coefft.begin(); c != coefft.end(); ++c)
		avgCoefft += *c;
	avgCoefft /= static_cast<float>(coefft.size());

	std::vector<unsigned char> texData(width * height * 3);

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
	Shader coefftShader(false, "PRTBake", false, false);
	GLuint tex_attrib = coefftShader.getAttribLoc("vTexCoord");
	GLuint coefft_attrib = coefftShader.getAttribLoc("vCoefft");

	// Pass data to GPU
	GLuint tex_vbo;
	glGenBuffers(1, &tex_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, tex_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * data.t.size(), data.t.data(), GL_STATIC_DRAW);
	GLuint coefft_vbo;
	glGenBuffers(1, &coefft_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, coefft_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * coefft.size(), coefft.data(), GL_STATIC_DRAW);
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
	glClearColor(avgCoefft.x, avgCoefft.y, avgCoefft.z, 1.0f);
	glDisable(GL_CULL_FACE);
	glClear(GL_COLOR_BUFFER_BIT);

	coefftShader.use();
	glEnableVertexAttribArray(tex_attrib);
	glEnableVertexAttribArray(coefft_attrib);

	glBindVertexBuffer(0, tex_vbo, 0, sizeof(glm::vec2));
	glVertexAttribFormat(tex_attrib, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(tex_attrib, 0);
	glBindVertexBuffer(1, coefft_vbo, 0, sizeof(glm::vec3));
	glVertexAttribFormat(coefft_attrib, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(coefft_attrib, 1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Render
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(data.e.size()), GL_UNSIGNED_SHORT, 0);

	// Rendering cleanup
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(tex_attrib);
	glDisableVertexAttribArray(coefft_attrib);
	glDeleteBuffers(1, &tex_vbo);
	glDeleteBuffers(1, &coefft_vbo);
	glDeleteBuffers(1, &elem_ebo);
	glUseProgram(0);
	//Restore old state
	if(faceCull == GL_TRUE) glEnable(GL_CULL_FACE);
	else glDisable(GL_CULL_FACE);
	glClearColor(clearCol[0], clearCol[1], clearCol[2], clearCol[3]);

	// Pull rendered image from GPU
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, width, height, GL_RGB, GL_BYTE, texData.data());

	SOIL_save_image
		(
			texFilename.c_str(),
			SOIL_SAVE_TYPE_BMP,
			width, height, 3,
			texData.data()
		);

	// More cleanup
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &frame);
	glDeleteRenderbuffers(1, &render); 
}

void PRTMesh::writeTransferToTextures(
	const std::vector<std::vector<glm::vec3>>& transfer,
	const MeshData& data,
	const std::string& prebakedFilename,
	std::vector<std::string>& coefftFilenames,
	int width, int height)
{
	for(unsigned c = 0; c < transfer[0].size(); ++c)
	{
		std::string texName = prebakedFilename + ".coefft" +
			std::to_string(static_cast<long long>(c)) + ".bmp";
		coefftFilenames.push_back(texName);
	}

	for(unsigned i = 0; i < coefftFilenames.size(); ++i)
	{
		std::vector<glm::vec3> currCoefft;
		for(auto v = transfer.begin(); v != transfer.end(); ++v)
			currCoefft.push_back((*v)[i]);

		renderCoefftToTexture(
			currCoefft,
			coefftFilenames[i],
			data, width, height);
	}
}

glm::vec3 PRTMesh::texLookup(
	unsigned char* image, 
	const glm::vec2& uv,
	int width, int height, int channels)
{
	int u = static_cast<int>(uv.x * width );
	int v = static_cast<int>(uv.y * height);

	int index = (u + v*width)*channels;
	float r = static_cast<float>(image[index    ]) / 255.0f;
	float g = static_cast<float>(image[index + 1]) / 255.0f;
	float b = static_cast<float>(image[index + 2]) / 255.0f;

	return glm::vec3(r, g, b);
}

void PRTMesh::init(
	const std::vector<PRTMeshVertex>& mesh,
	const std::vector<GLushort>& elems)
{
	numElems = elems.size();
	shader->setTexUnit(arrTex->getTexUnit());

	glGenBuffers(1, &v_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, v_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PRTMeshVertex) * mesh.size(),
		mesh.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &e_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * elems.size(),
		elems.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	v_attrib = shader->getAttribLoc("vPosition");
	t_attrib = shader->getAttribLoc("vTexCoord");
}

void PRTMesh::render()
{
	if(!scene) return;
	
	shader->setModelToWorld(modelToWorld);

	shader->setTexUnit(arrTex->getTexUnit());

	shader->use();
	glEnableVertexAttribArray(v_attrib);
	glEnableVertexAttribArray(t_attrib);

	glBindVertexBuffer(0, v_vbo, 0, sizeof(PRTMeshVertex));
	glVertexAttribFormat(v_attrib, 4, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(v_attrib, 0);
	glVertexAttribFormat(t_attrib, 2, GL_FLOAT, GL_FALSE, offsetof(PRTMeshVertex, t));
	glVertexAttribBinding(t_attrib, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_ebo);

	glDrawElements(GL_TRIANGLES, (GLsizei) numElems, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(v_attrib);
	glDisableVertexAttribArray(t_attrib);

	glUseProgram(0);
}
