#include "PRTMesh.hpp"

PRTMesh::PRTMesh(
	const std::string& filename,
	const Material& material,
	PRTMode mode, int sqrtNSamples,
	int nBands,
	Shader* shader, 
	int nBounces)
	:Renderable(false), shader(shader)
{
	std::string prebakedFilename = genPrebakedFilename(filename, mode, nBands);

	if(fileExists(prebakedFilename))
		readPrebakedFile(
			verts, elems, 
			transfer, nBands*nBands, 
			prebakedFilename);

	else
	{
		MeshData data = Mesh::loadSceneFile(filename, material);
		elems = data.e;
		bake(data, mode, nBands, sqrtNSamples, verts, transfer, nBounces);
		writePrebakedFile(verts, elems, transfer, prebakedFilename);
	}

	init();
}

PRTMesh::PRTMesh(
	const std::vector<std::string>& filenames,
	const std::vector<Material>& materials,
	PRTMode mode, int sqrtNSamples,
	int nBands,
	Shader* shader, 
	int nBounces)
	:Renderable(false), shader(shader)
{
	std::string prebakedFilename = "";
	for(auto n = filenames.begin(); n != filenames.end(); ++n)
		prebakedFilename += *n;
	prebakedFilename = genPrebakedFilename(prebakedFilename, mode, nBands);

	if(fileExists(prebakedFilename))
		readPrebakedFile(
			verts, elems, 
			transfer, nBands*nBands, 
			prebakedFilename);

	else
	{
		MeshData data = Mesh::loadSceneFiles(filenames, materials);
		elems = data.e;
		bake(data, mode, nBands, sqrtNSamples, verts, transfer, nBounces);
		writePrebakedFile(verts, elems, transfer, prebakedFilename);
	}

	init();
}

void PRTMesh::render()
{
	//Calculate vertex colors.
	#pragma omp parallel for	
	for(int i = 0; i < static_cast<int>(transfer.size()); ++i)
		colors[i] = scene->getSHLitColor(transfer[i]);

	//Send new colors to GPU.
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 
		sizeof(glm::vec4) * colors.size(), colors.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Render.
	shader->use();

	glEnableVertexAttribArray(vert_attrib);
	glEnableVertexAttribArray(color_attrib);

	glBindVertexBuffer(0, verts_vbo, 0, sizeof(glm::vec4));
	glVertexAttribFormat(vert_attrib, 4, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(vert_attrib, 0);

	glBindVertexBuffer(1, colors_vbo, 0, sizeof(glm::vec4));
	glVertexAttribFormat(color_attrib, 4, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(color_attrib, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elems_vbo);

	glDrawElements(GL_TRIANGLES, (GLsizei) elems.size(), GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(vert_attrib);
	glDisableVertexAttribArray(color_attrib);

	glUseProgram(0);
}

void PRTMesh::init()
{
	for(unsigned i = 0; i < verts.size(); ++i)
		colors.push_back(glm::vec4(0.0f));

	glGenBuffers(1, &verts_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, verts_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * verts.size(),
		verts.data(), GL_STATIC_DRAW);
	glGenBuffers(1, &colors_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * colors.size(),
		colors.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &elems_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elems_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * elems.size(),
		elems.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	vert_attrib  = shader->getAttribLoc("vPosition");
	color_attrib = shader->getAttribLoc("vColor");
}

void PRTMesh::bake(const MeshData& data,
	PRTMode mode, int nBands, int sqrtNSamples,
	std::vector<glm::vec4>& verts,
	std::vector<std::vector<glm::vec3>>& transfer, 
	int nBounces)
{
	verts.resize(data.v.size());

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
			glm::vec4 vert = data.v[i];
			verts.push_back(vert);

			std::vector<glm::vec3> coeffts;

			if(mode == UNSHADOWED)
				coeffts = SH::shProject(sqrtNSamples, nBands, 
					[&data, &i](float theta, float phi) -> glm::vec3 
						{
							glm::vec3 dir
								(
								sin(theta) * cos(phi),
								sin(theta) * sin(phi),
								cos(theta)
								);
							dir = glm::normalize(dir);
							glm::vec3 norm = glm::normalize(data.n[i]);

							Material mat = data.M[data.m[i]];
							glm::vec3 diffuse(mat.diffuse);

							float proj = glm::dot(dir, norm);
							proj = (proj > 0.0f ? proj : 0.0f);

							return diffuse * proj;
						}
					);

			else // mode == SHADOWED || mode == INTERREFLECTED
				coeffts = SH::shProject(sqrtNSamples, nBands, 
					[&data, &i](float theta, float phi) -> glm::vec3 
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

							Material mat = data.M[data.m[i]];
							glm::vec3 diffuse(mat.diffuse);

							return proj * diffuse;
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
		interreflect(data, nBands, sqrtNSamples, nBounces, verts, transfer);
	}
}

void PRTMesh::interreflect(
	const MeshData& data,
	int nBands, int sqrtNSamples, int nBounces,
	const std::vector<glm::vec4>& verts,
	std::vector<std::vector<glm::vec3>>& transfer)
{
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
		int nVerts = static_cast<int>(verts.size());

		#pragma omp parallel private(tid)
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

						// Find average diffuse color over triangle
						glm::vec4 avgDiffuse = 
							(1-(tu+tv)) * data.M[data.m[data.e[closestTri]]].diffuse +
							tu * data.M[data.m[data.e[closestTri + 1]]].diffuse +
							tv * data.M[data.m[data.e[closestTri + 2]]].diffuse;

						for(int c = 0; c < nBands*nBands; ++c)
						{
							glm::vec3 avgPrevBounce = 
								((1-(tu+tv)) * prevBounce[data.e[closestTri]][c] +
								tu * prevBounce[data.e[closestTri + 1]][c] +
								tv * prevBounce[data.e[closestTri + 2]][c]);

							currBounce[i][c] += 
								glm::dot(data.n[i], dir) * glm::vec3(avgDiffuse) * avgPrevBounce;
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
}

std::string PRTMesh::genPrebakedFilename(
	const std::string& filename,
	PRTMode mode,
	int nBands
	)
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
	const std::vector<glm::vec4>& verts,
	const std::vector<GLushort>& elems,
 	const std::vector<std::vector<glm::vec3>>& transfer,
 	const std::string& filename)
{
	std::ofstream file(filename);

	file << "Vertices" << std::endl;

	for(auto v = verts.begin(); v != verts.end(); ++v)
		file
			<< v->x << " "
			<< v->y << " "
			<< v->z << " " 
			<< v->w << std::endl;

	file << "Elements" << std::endl;

	for(auto e = elems.begin(); e != elems.end(); ++e)
		file << *e << std::endl;

	file << "Transfer Coeffts" << std::endl;

	for(auto v = transfer.begin(); v != transfer.end(); ++v)
	{
		for(auto c = v->begin(); c != v->end(); ++c)
		{
			file 
				<< c->x << " "
				<< c->y << " "
				<< c->z << " ";
		}
		file << std::endl;
	}

	file.close();
}

void PRTMesh::readPrebakedFile(
	std::vector<glm::vec4>& verts,
	std::vector<GLushort>& elems,
 	std::vector<std::vector<glm::vec3>>& transfer,
 	int nCoeffts,
 	const std::string& filename)
{
	std::ifstream file(filename);

	if(!file) throw(new MeshFileException);

	char ignore[10];

	file.getline(ignore, 10); //Throw the "Vertices" line.

	float next;

	while(file >> next)
	{
		glm::vec4 vert;

		vert.x = next;
		file >> vert.y;
		file >> vert.z;
		file >> vert.w;

		verts.push_back(vert);
	}

	file.clear();
	file.getline(ignore, 10); //Throw the "Elements" line.

	int elem;
	while(file >> elem)
		elems.push_back(static_cast<GLushort>(elem));

	file.clear();
	file.getline(ignore, 10); //Throw the "Transfer Coeffts" line.

	for(auto v = verts.begin(); v != verts.end(); ++v)
	{
		std::vector<glm::vec3> coeffts;

		for(int i = 0; i < nCoeffts; ++i)
		{
			glm::vec3 coefft;
			file >> coefft.x;
			file >> coefft.y;
			file >> coefft.z;
			coeffts.push_back(coefft);
		}

		transfer.push_back(coeffts);
	}
}
