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

		std::cout << "Loading scene from file: " << filename << "\n";

		std::vector<MeshData> data;

		for(int i = 0; i < (int) scene->mNumMeshes; ++i)
		{
			aiMesh* mesh = scene->mMeshes[i];

			MeshData d;

			std::cout << "> Mesh " << i << " of "
				<< mesh->mNumVertices << " vertices, "
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

	bool isNAN(float f)
	{
		return (f != f); //Note that NAN != NAN.
	}

	bool isNAN(glm::vec3 v)
	{
		return (
			isNAN(v.x) ||
			isNAN(v.y) ||
			isNAN(v.z) 
			);
	}
}

std::vector<Mesh*> Mesh::loadFile(const std::string& filename,
	LightShader* _shader)
{
	std::vector<Mesh*> meshes;
	std::vector<MeshData> data = loadFileData(filename);

	for(auto i = data.begin(); i != data.end(); ++i)
		meshes.push_back(new Mesh(*i, _shader));

	return meshes;
}

Mesh::Mesh(const MeshData& d, LightShader* _shader)
	:Solid(_shader)
{
	numElems = d.e.size();

	std::vector<MeshVertex> vertexBuffer;
	for(GLuint i = 0; i < d.v.size(); ++i)
	{
		MeshVertex vert;
		vert.v = d.v[i]; vert.n = d.n[i];
		vertexBuffer.push_back(vert);
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
}

void Mesh::render()
{
	if(!scene) return;
	
	shader->setModelToWorld(modelToWorld);
	shader->setMaterial(material);

	shader->use();
	glEnableVertexAttribArray(v_attrib);
	glEnableVertexAttribArray(n_attrib);

	glBindVertexBuffer(0, v_vbo, 0, sizeof(MeshVertex));
	glVertexAttribFormat(v_attrib, 4, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(v_attrib, 0);
	glVertexAttribFormat(n_attrib, 3, GL_FLOAT, GL_FALSE, offsetof(MeshVertex, n));
	glVertexAttribBinding(n_attrib, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_vbo);

	glDrawElements(GL_TRIANGLES, (GLsizei) numElems, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(v_attrib);
	glDisableVertexAttribArray(n_attrib);
}

std::vector<DiffPRTMesh*> DiffPRTMesh::loadFile(
	DiffPRTMode mode,
	const std::string& filename,
	SHShader* _shader)
{
	if(mode != UNSHADOWED && mode != SHADOWED && mode != INTERREFLECTED)
		throw(new BadPRTModeException);

	std::cout << "Attempting to load scene file " + filename << std::endl;

	std::vector<DiffPRTMesh*> meshes;

	/* Check for file with precomputed coeffts */
	/* Precomputed filenames are of the format:
	 * originalname.extension.prt[ds][usi][0-9]+
	 * Where d is diffuse, s specular
	 *		 u is unshadowed, s shadowed, i interreflected
	 *       The terminating number indicates the no. of SH
	 *       bands used in the approximation.
	 */
	std::string prtFilename;

	if(UNSHADOWED)
		prtFilename = filename + ".prtdu" +
			std::to_string(static_cast<long long>(GC::nSHBands)); 
	else if (SHADOWED)
		prtFilename = filename + ".prtds" +
			std::to_string(static_cast<long long>(GC::nSHBands));
	else // INTERREFLECTED
		;

	std::ifstream prtFile(prtFilename); 

	/* File exists, so load data from it */
	if(prtFile.is_open()) 
	{
		std::cout << "Precomputed file found." << std::endl;
		std::cout << "Loading from precomputed file " + prtFilename << std::endl;

		while(prtFile.good())
		{
			std::vector<PRTMeshVertex> vertexBuffer;
			std::vector<GLushort> elemBuffer;

			char ignore[10]; 

			prtFile.getline(ignore, 10); // Throw the "Mesh N" line.
			prtFile.getline(ignore, 10); // Throw the "Vertices" line.

			/* Get vertexBuffer */
			float next;
			while(prtFile >> next)
			{
				PRTMeshVertex vert;
				vert.v.x = next;
				prtFile >> vert.v.y;
				prtFile >> vert.v.z;
				prtFile >> vert.v.w;
				for(int c = 0; c < GC::nSHCoeffts; ++c)
				{
					prtFile >> vert.s[c].x;
					prtFile >> vert.s[c].y;
					prtFile >> vert.s[c].z;
					prtFile >> vert.s[c].w;
				}

				vertexBuffer.push_back(vert);
			}
			
			prtFile.clear();

			/* Get elemBuffer */
			prtFile.getline(ignore, 10); // Throw the "Elements" line.
			int elem;
			while(prtFile >> elem)
				elemBuffer.push_back(static_cast<GLushort>(elem));

			meshes.push_back(new DiffPRTMesh(vertexBuffer, elemBuffer, _shader));
			std::cout << "> Mesh " << meshes.size() - 1 
				<< " of " << vertexBuffer.size() << " vertices, " 
				<< elemBuffer.size() << " elements." << std::endl;
		}

		prtFile.close();
	}

	/* No such file exists, so load from regular file & compute coeffts */
	else
	{
		std::cout << "No precomputed file \"" << prtFilename << "\" was found." << std::endl;

		std::vector<MeshData> data = loadFileData(filename);

		std::ofstream outFile(prtFilename);

		std::cout << "> Calculating transfer function coefficients (may take some time)..." << std::endl;

		for(GLuint m = 0; m < data.size(); ++m)
		{
			std::vector<PRTMeshVertex> vertBuffer = 
				computeVertBuffer(data[m], mode);
			meshes.push_back(new DiffPRTMesh(vertBuffer, data[m].e, _shader));

			outFile << "Mesh " << std::to_string(static_cast<long long>(m)) << std::endl;
			outFile << "Vertices" << std::endl;

			for(auto i = vertBuffer.begin(); i != vertBuffer.end(); ++i)
			{
				outFile << (*i).v.x << " " 
					<< (*i).v.y << " "
					<< (*i).v.z << " " 
					<< (*i).v.w << std::endl;

				for(int c = 0; c < GC::nSHCoeffts; ++c)
					outFile << (*i).s[c].x << " " 
					<< (*i).s[c].y << " " 
					<< (*i).s[c].z << " " 
					<< (*i).s[c].w << std::endl;
			}

			outFile << "Elements" << std::endl;

			for(auto i = data[m].e.begin(); i != data[m].e.end(); ++i)
				outFile << (*i) << std::endl;
		}

		outFile.close();
	}

	return meshes;
}


DiffPRTMesh::DiffPRTMesh(const std::vector<PRTMeshVertex>& vertBuffer,
	const std::vector<GLushort>& elemBuffer, SHShader* _shader)
	:Solid(_shader)
{
	numElems = elemBuffer.size();

	glGenBuffers(1, &v_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, v_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PRTMeshVertex) * vertBuffer.size(),
		vertBuffer.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &e_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * elemBuffer.size(),
		elemBuffer.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	v_attrib = shader->getAttribLoc("vPos");
	s_attrib = shader->getAttribLoc("transferCoeffts");
}

std::vector<PRTMeshVertex> DiffPRTMesh::computeVertBuffer(
	const MeshData& d, DiffPRTMode mode)
{
	std::vector<PRTMeshVertex> vertBuffer(d.v.size());

	#pragma omp parallel for
	for(int i = 0; i < static_cast<int>(d.v.size()); ++i)
	{
		PRTMeshVertex vert;
		vert.v = d.v[i];

		std::vector<glm::vec4> coeffts;

		if(mode == UNSHADOWED)
			coeffts = SH::shProject(GC::sqrtSHSamples, GC::nSHBands, 
				[&d, &i](double theta, double phi) -> glm::vec3 
					{
						glm::vec3 dir
							(
							sin(theta) * cos(phi),
							sin(theta) * sin(phi),
							cos(theta)
							);
						dir = glm::normalize(dir);
						glm::vec3 norm = glm::normalize(d.n[i]);
						double proj = glm::dot(dir, norm);
						proj = (proj > 0.0 ? proj : 0.0);
						return glm::vec3(proj, proj, proj);
					}
				);
		else if(mode == SHADOWED)
			coeffts = SH::shProject(GC::sqrtSHSamples, GC::nSHBands, 
				[&d, &i](double theta, double phi) -> glm::vec3 
					{
						bool intersect = false;
						glm::vec3 dir
							(
							sin(theta) * cos(phi),
							sin(theta) * sin(phi),
							cos(theta)
							);
						dir = glm::normalize(dir);
						glm::vec3 norm = glm::normalize(d.n[i]);
						double proj = glm::dot(dir, norm);
						if(proj <= 0.0f) return glm::vec3(0.0, 0.0, 0.0);

						// For each triangle in mesh
						for(size_t e = 0; e < d.e.size(); e += 3)
						{
							// Find triangle vertices
							glm::vec3 ta = glm::vec3(d.v[d.e[e]]);
							glm::vec3 tb = glm::vec3(d.v[d.e[e+1]]);
							glm::vec3 tc = glm::vec3(d.v[d.e[e+2]]);

							// Check for intersection
							if(triangleRayIntersect(
								ta, tb, tc, glm::vec3(d.v[i]), dir))
							{
								intersect = true;
								break;
							}
						}
						// Light is blocked, return 0.
						if(intersect) return glm::vec3(0.0, 0.0, 0.0);
						// Light not occluded.

						return glm::vec3(proj, proj, proj);
					}
				);

		else // INTERREFLECTED
			;

		for(GLuint c = 0; c < coeffts.size(); ++c)
			vert.s[c] = coeffts[c];

		vertBuffer[i] = vert;
	}

	return vertBuffer;
}

void DiffPRTMesh::render()
{
	if(!scene) return;
	
	shader->setModelToWorld(modelToWorld);

	shader->use();
	glEnableVertexAttribArray(v_attrib);
	for(int c = 0; c < GC::nSHCoeffts; ++c)
		glEnableVertexAttribArray(s_attrib + c);

	glBindVertexBuffer(0, v_vbo, 0, sizeof(PRTMeshVertex));
	glVertexAttribFormat(v_attrib, 4, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(v_attrib, 0);
	for(int c = 0; c < GC::nSHCoeffts; ++c)
	{
		glVertexAttribFormat(s_attrib + c, 4, GL_FLOAT,
			GL_FALSE, offsetof(PRTMeshVertex, s) + c * sizeof(glm::vec4));
		glVertexAttribBinding(s_attrib + c, 0);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_vbo);

	glDrawElements(GL_TRIANGLES, (GLsizei) numElems, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(v_attrib);
	for(int c = 0; c < GC::nSHCoeffts; ++c)
		glDisableVertexAttribArray(s_attrib + c);
}

std::vector<AOMesh*> AOMesh::loadFile(
		const std::string& filename,
		AOShader* _shader)
{
	std::cout << "Attempting to load scene file " + filename << std::endl;

	std::vector<AOMesh*> meshes;

	/* Check for file with precomputed coeffts */
	/* Precomputed filenames are of the format:
	 * originalname.extension.ao
	 */
	std::string aoFilename = filename + ".ao";

	std::ifstream aoFile(aoFilename); 

	/* File exists, so load data from it */
	if(aoFile.is_open()) 
	{
		std::cout << "Precomputed file found." << std::endl;
		std::cout << "Loading from precomputed file " + aoFilename << std::endl;

		/* Load each mesh in turn */
		while(aoFile.good())
		{
			std::vector<AOMeshVertex> vertBuffer;
			std::vector<GLushort> elemBuffer;

			char ignore[10];
			aoFile.getline(ignore, 10); //Throw the "Mesh N" line.
			aoFile.getline(ignore, 10); //Throw the "Vertices" line.

			float next;

			while(aoFile >> next)
			{
				AOMeshVertex vert;
				vert.v.x = next;
				aoFile >> vert.v.y;
				aoFile >> vert.v.z;
				aoFile >> vert.v.w;
				aoFile >> vert.bentN.x;
				aoFile >> vert.bentN.y;
				aoFile >> vert.bentN.z;
				aoFile >> vert.occl;

				vertBuffer.push_back(vert);
			}

			aoFile.clear();

			aoFile.getline(ignore, 10); //Throw the "Elements" line.

			int elem;
			while(aoFile >> elem)
				elemBuffer.push_back(static_cast<GLushort>(elem));

			meshes.push_back(new AOMesh(vertBuffer, elemBuffer, _shader));

			std::cout << "> Mesh " << meshes.size() - 1 
				<< " of " << vertBuffer.size() << " vertices, " 
				<< elemBuffer.size() << " elements." << std::endl;
		}

		aoFile.close();
	}

	/* No precomputed file found. */
	else
	{
		std::cout << "No precomputed file \"" << aoFilename << "\" was found." << std::endl;

		std::vector<MeshData> data = loadFileData(filename);

		std::ofstream outFile(aoFilename);

		std::cout << "> Calculating AO coefficients & bent normals (may take some time)..." << std::endl;

		for(GLuint m = 0; m < data.size(); ++m)
		{
			std::vector<AOMeshVertex> vertBuffer = computeVertBuffer(data[m]);
			meshes.push_back(new AOMesh(vertBuffer, data[m].e, _shader));

			outFile << "Mesh " << std::to_string(static_cast<long long>(m)) << std::endl;
			outFile << "Vertices" << std::endl;

			for(auto i = vertBuffer.begin(); i != vertBuffer.end(); ++i)
			{
				outFile << (*i).v.x << " " 
					<< (*i).v.y << " " 
					<< (*i).v.z << " " 
					<< (*i).v.w << std::endl;

				outFile << (*i).bentN.x << " " 
					<< (*i).bentN.y << " " 
					<< (*i).bentN.z << std::endl;

				outFile << (*i).occl << std::endl;
			}

			outFile << "Elements" << std::endl;

			for(auto i = data[m].e.begin(); i != data[m].e.end(); ++i)
				outFile << (*i) << std::endl;
		}

		outFile.close();

		std::cout << "> Done!" << std::endl;
	}

	return meshes;
}

std::vector<AOMeshVertex> AOMesh::computeVertBuffer(const MeshData& d)
{
	std::vector<AOMeshVertex> vertBuffer(d.v.size());

	#pragma omp parallel for
	for(int i = 0; i < static_cast<int>(d.v.size()); ++i)
	{
		vertBuffer[i].v = d.v[i];
		vertBuffer[i].bentN = glm::vec3(0.0f);
		vertBuffer[i].occl = 0.0f;

		double sqrSize = 1.0 / GC::sqrtAOSamples;


		/* Perform stratified sampling in the hemisphere around the norm */
		/* Sample over whole sphere first */
		for(int x = 0; x < GC::sqrtAOSamples; ++x)
			for(int y = 0; y < GC::sqrtAOSamples; ++y)
			{
				double u = (x * sqrSize);
				double v = (y * sqrSize);
				double theta = acos((2 * u) - 1);
				double phi = (2 * PI_d * v);

				glm::vec3 dir
					(
					sin(theta) * cos(phi),
					sin(theta) * sin(phi),
					cos(theta)
					); 

				/* Continue if dir is not in hemisphere around norm */
				if(glm::dot(dir, d.n[i]) < 0.0f) continue;

				/* Check for intersection */
				bool intersect = false;

				for(size_t t = 0; t < d.e.size(); t += 3)
				{
					// Find triangle vertices
					glm::vec3 ta = glm::vec3(d.v[d.e[t]]);
					glm::vec3 tb = glm::vec3(d.v[d.e[t+1]]);
					glm::vec3 tc = glm::vec3(d.v[d.e[t+2]]);

					if(triangleRayIntersect(ta, tb, tc, glm::vec3(d.v[i]), dir))
					{
						intersect = true;
						break; // No need to check other triangles.
					}
				}

				if(!intersect)
				{
					vertBuffer[i].bentN += dir;
					vertBuffer[i].occl += 1.0f;
				}
			}

		/* Normalize bent norm and occlusion coefft */
		vertBuffer[i].bentN = glm::normalize(vertBuffer[i].bentN);
		if(isNAN(vertBuffer[i].bentN))
			vertBuffer[i].bentN = d.n[i]; // Give original vector as fallback.

		vertBuffer[i].occl /= PI * (GC::nAOSamples);
	}

	return vertBuffer;
}

AOMesh::AOMesh(const std::vector<AOMeshVertex>& vertBuffer,
	const std::vector<GLushort>& elemBuffer, AOShader* _shader)
	:Solid(_shader)
{
	numElems = elemBuffer.size();

	glGenBuffers(1, &v_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, v_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(AOMeshVertex) * vertBuffer.size(),
		vertBuffer.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &e_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * elemBuffer.size(),
		elemBuffer.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	v_attrib = shader->getAttribLoc("vPos");
	bentN_attrib = shader->getAttribLoc("bentN");
	occl_attrib = shader->getAttribLoc("occl");
}

void AOMesh::render()
{
	if(!scene) return;
	
	shader->setModelToWorld(modelToWorld);
	shader->setMaterial(material);

	shader->use();
	glEnableVertexAttribArray(v_attrib);
	glEnableVertexAttribArray(bentN_attrib);
	glEnableVertexAttribArray(occl_attrib);

	glBindVertexBuffer(0, v_vbo, 0, sizeof(AOMeshVertex));
	glVertexAttribFormat(v_attrib, 4, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(v_attrib, 0);
	glVertexAttribFormat(bentN_attrib, 3, GL_FLOAT, GL_FALSE, 
		offsetof(AOMeshVertex, bentN));
	glVertexAttribBinding(bentN_attrib, 0);
	glVertexAttribFormat(occl_attrib, 1, GL_FLOAT, GL_FALSE, 
		offsetof(AOMeshVertex, occl));
	glVertexAttribBinding(occl_attrib, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_vbo);

	glDrawElements(GL_TRIANGLES, (GLsizei) numElems, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(v_attrib);
	glDisableVertexAttribArray(bentN_attrib);
	glDisableVertexAttribArray(occl_attrib);
}
