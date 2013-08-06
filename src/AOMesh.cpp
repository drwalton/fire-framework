#include "AOMesh.hpp"

AOMesh::AOMesh(
	const std::string& filename,
	const Material& material,
	AOShader* shader, int sqrtNSamples)
	:Renderable(false), shader(shader)
{
	mats.push_back(material);

	std::vector<AOMeshVertex> mesh;
	std::vector<GLushort> elems;

	std::string prebakedFilename = filename + ".ao";
	if(fileExists(prebakedFilename))
		readPrebakedFile(mesh, elems, mats, prebakedFilename);
	else
	{
		MeshData data = Mesh::loadSceneFile(filename, material);
		elems = data.e;
		bake(data, sqrtNSamples, mesh, elems);
		writePrebakedFile(mesh, elems, mats, prebakedFilename);
	}

	init(mesh, elems);
}

AOMesh::AOMesh(
	const std::vector<std::string>& filenames,
	const std::vector<Material>& materials,
	AOShader* shader, int sqrtNSamples)
	:Renderable(false), shader(shader)
{
	mats = materials;

	std::vector<AOMeshVertex> mesh;
	std::vector<GLushort> elems;

	std::string prebakedFilename = "";
	for(auto i = filenames.begin(); i != filenames.end(); ++i)
		prebakedFilename += *i;
	prebakedFilename += ".ao";

	if(fileExists(prebakedFilename))
		readPrebakedFile(mesh, elems, mats, prebakedFilename);
	else
	{
		MeshData data = Mesh::loadSceneFiles(filenames, mats);
		elems = data.e;
		numElems = data.e.size();
		bake(data, sqrtNSamples, mesh, elems);
		writePrebakedFile(mesh, elems, mats, prebakedFilename);
	}

	init(mesh, elems);
}

void AOMesh::init(
		const std::vector<AOMeshVertex>& mesh,
		const std::vector<GLushort>& elems)
{
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
	m_attrib = shader->getAttribLoc("vMatIndex");
}

void AOMesh::render()
{
	if(!scene) return;
	
	shader->setModelToWorld(modelToWorld);
	static_cast<AOShader*>(shader)->setMaterials(mats);

	shader->use();
	glEnableVertexAttribArray(v_attrib);
	glEnableVertexAttribArray(n_attrib);
	glEnableVertexAttribArray(m_attrib);
	glEnableVertexAttribArray(occl_attrib);

	glBindVertexBuffer(0, v_vbo, 0, sizeof(AOMeshVertex));
	glVertexAttribFormat(v_attrib, 4, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(v_attrib, 0);
	glVertexAttribFormat(n_attrib, 3, GL_FLOAT, GL_FALSE, offsetof(AOMeshVertex, n));
	glVertexAttribBinding(n_attrib, 0);
	glVertexAttribFormat(m_attrib, 1, GL_INT, GL_FALSE, offsetof(AOMeshVertex, m));
	glVertexAttribBinding(m_attrib, 0);
	glVertexAttribFormat(occl_attrib, 1, GL_FLOAT, GL_FALSE, offsetof(AOMeshVertex, occl));
	glVertexAttribBinding(occl_attrib, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_vbo);

	glDrawElements(GL_TRIANGLES, (GLsizei) numElems, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(v_attrib);
	glDisableVertexAttribArray(n_attrib);
	glDisableVertexAttribArray(m_attrib);
	glDisableVertexAttribArray(occl_attrib);

	glUseProgram(0);
}

void AOMesh::bake(
	const MeshData& data,
	int sqrtNSamples,
	std::vector<AOMeshVertex>& mesh, 
	const std::vector<GLushort>& elems)
{
	mesh.resize(data.v.size());

	#pragma omp parallel for
	for(int i = 0; i < static_cast<int>(data.v.size()); ++i)
	{
		mesh[i].v = data.v[i];
		mesh[i].n = glm::vec3(0.0f);
		mesh[i].occl = 0.0f;

		double sqrSize = 1.0 / sqrtNSamples;

		/* Perform stratified sampling in the hemisphere around the norm */
		/* Sample over whole sphere first */
		for(int x = 0; x < sqrtNSamples; ++x)
			for(int y = 0; y < sqrtNSamples; ++y)
			{
				double u = (x * sqrSize);
				double v = (y * sqrSize);
				if(GC::jitterSamples)
				{
					u += randd(0, sqrSize);
					v += randd(0, sqrSize);
				}

				double theta = acos((2 * u) - 1);
				double phi = (2 * PI_d * v);

				glm::vec3 dir
					(
					sin(theta) * cos(phi),
					sin(theta) * sin(phi),
					cos(theta)
					); 

				/* Continue if dir is not in hemisphere around norm */
				if(glm::dot(dir, data.n[i]) < 0.0f) continue;

				/* Check for intersection */
				bool intersect = false;

				for(unsigned t = 0; t < data.e.size(); t += 3)
				{
					// Find triangle vertices
					glm::vec3 ta = glm::vec3(data.v[data.e[t]]);
					glm::vec3 tb = glm::vec3(data.v[data.e[t+1]]);
					glm::vec3 tc = glm::vec3(data.v[data.e[t+2]]);

					if(triangleRayIntersect(ta, tb, tc, glm::vec3(data.v[i]), dir))
					{
						intersect = true;
						break; // No need to check other triangles.
					}
				}

				if(!intersect)
				{
					mesh[i].n += dir;
					mesh[i].occl += 1.0f;
				}
			}

		/* Normalize bent norm and occlusion coefft */
		if(!(abs(mesh[i].n.x) < EPS && 
			 abs(mesh[i].n.x) < EPS && 
			 abs(mesh[i].n.z) < EPS))
			 mesh[i].n = glm::normalize(mesh[i].n);

		mesh[i].occl /= PI * (sqrtNSamples * sqrtNSamples);
	}
}

void AOMesh::writePrebakedFile(
	const std::vector<AOMeshVertex>& mesh,
	const std::vector<GLushort> & elems,
 	const std::vector<Material>& mats,
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
			<< v->m << std::endl;
		file 
			<< v->occl << std::endl;
	}

	file << "Elements" << std::endl;

	for(auto e = elems.begin(); e != elems.end(); ++e)
		file << *e << std::endl;

	file << "Materials" << std::endl;

	for(auto m = mats.begin(); m != mats.end(); ++m)
	{
		file
			<< m->ambient.x << " "
			<< m->ambient.y << " "
			<< m->ambient.z << " "
			<< m->ambient.w << std::endl;
		file
			<< m->diffuse.x << " "
			<< m->diffuse.y << " "
			<< m->diffuse.z << " "
			<< m->diffuse.w << std::endl;
		file
			<< m->specular.x << " "
			<< m->specular.y << " "
			<< m->specular.z << " "
			<< m->specular.w << std::endl;
		file << m->exponent << std::endl;
	}

	file.close();
}

void AOMesh::readPrebakedFile(
	std::vector<AOMeshVertex>& mesh,
	std::vector<GLushort>& elems,
 	std::vector<Material>& mats,
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

		file >> vert.m;

		file >> vert.occl;

		mesh.push_back(vert);
	}

	file.clear();
	file.getline(ignore, 10); //Throw the "Elements" line.

	int elem;
	while(file >> elem)
		elems.push_back(static_cast<GLushort>(elem));

	file.clear();
	file.getline(ignore, 10); //Throw the "Materials" line.

	while(file >> next)
	{
		Material mat;

		mat.ambient.x = next;
		file >> mat.ambient.y;
		file >> mat.ambient.z;
		file >> mat.ambient.w;

		file >> mat.diffuse.x;
		file >> mat.diffuse.y;
		file >> mat.diffuse.z;
		file >> mat.diffuse.w;

		file >> mat.specular.x;
		file >> mat.specular.y;
		file >> mat.specular.z;
		file >> mat.specular.w;

		file >> mat.exponent;

		mats.push_back(mat);
	}

	file.close();
}
