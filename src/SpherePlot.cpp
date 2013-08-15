#include "SpherePlot.hpp"

#include "Shader.hpp"

SpherePlotMesh SpherePlot::genMesh(const std::vector<SphereSample>& samples)
{
	SpherePlotMesh mesh;

	// Add vertex positions.
	for(auto i = samples.begin(); i != samples.end(); ++i)
	{
		SpherePlotVertex v;

		glm::vec4 dir(
			sin(i->theta) * cos(i->phi),
			sin(i->theta) * sin(i->phi),					
			cos(i->theta), 
			0.0f);
		glm::vec4 origin(0.0f, 0.0f, 0.0f, 1.0f);
		
		v.pos = origin + dir * abs(i->val);
		v.positive = i->val >= 0.0 ? 1.0f : 0.0f;
		mesh.v.push_back(v);
	}

	int sqrtNSamples = static_cast<int>(
		sqrt(static_cast<double>(samples.size())));

	/* Add elements, norms. */
	for(unsigned v = 0; v < mesh.v.size(); ++v)
	{
		unsigned i = v % sqrtNSamples;
		unsigned j = v / sqrtNSamples;

		if(i == sqrtNSamples || j == sqrtNSamples) continue;

		/* Verts at each corner of a quad. */
		GLushort tlv, trv, blv, brv;
		tlv = i + j*sqrtNSamples;
		trv = i + 1 + j*sqrtNSamples;
		blv = i + (j + 1)*sqrtNSamples;
		brv = (i + 1) + (j + 1)*sqrtNSamples;

		/* Calculate norms. */
		glm::vec3 across(mesh.v[trv].pos - mesh.v[tlv].pos);
		glm::vec3   down(mesh.v[blv].pos - mesh.v[tlv].pos);
		glm::vec3 norm = glm::cross(down, across);
		if(abs(norm.x) < EPS && abs(norm.y) < EPS && abs(norm.z) < EPS)
			mesh.v[tlv].norm = glm::vec3(0.0f, 0.0f, 0.0f);
		else
			mesh.v[tlv].norm = glm::normalize(norm);

		/* Add elements. */
		mesh.e.push_back(blv);
		mesh.e.push_back(tlv);
		mesh.e.push_back(trv);

		mesh.e.push_back(trv);
		mesh.e.push_back(brv);
		mesh.e.push_back(blv);
	}

	return mesh;
}

void SpherePlot::uploadMeshToGPU(const SpherePlotMesh& mesh)
{
	numElems = static_cast<GLsizei>(mesh.e.size());

	glGenBuffers(1, &v_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, v_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SpherePlotVertex) * mesh.v.size(),
		mesh.v.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glGenBuffers(1, &e_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * mesh.e.size(),
		mesh.e.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	pos_attrib = shader->getAttribLoc("vPosition");
	norm_attrib = shader->getAttribLoc("vNorm");
	positive_attrib = shader->getAttribLoc("vPositive");
}

void SpherePlot::render()
{
	if(!scene) return;

	shader->setModelToWorld(modelToWorld);
	shader->use();

	glEnableVertexAttribArray(pos_attrib);
	glEnableVertexAttribArray(norm_attrib);
	glEnableVertexAttribArray(positive_attrib);

	glBindVertexBuffer(0, v_vbo, 0, sizeof(SpherePlotVertex));
	glVertexAttribFormat(pos_attrib, 4, GL_FLOAT, GL_FALSE,
		offsetof(SpherePlotVertex, pos));
	glVertexAttribBinding(pos_attrib, 0);
	glVertexAttribFormat(norm_attrib, 3, GL_FLOAT, GL_FALSE,
		offsetof(SpherePlotVertex, norm));
	glVertexAttribBinding(norm_attrib, 0);
	glVertexAttribFormat(positive_attrib, 1, GL_FLOAT, GL_FALSE,
		offsetof(SpherePlotVertex, positive));
	glVertexAttribBinding(positive_attrib, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e_vbo);
	
	glDrawElements(GL_TRIANGLES, numElems, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(pos_attrib);
	glDisableVertexAttribArray(norm_attrib);
	glDisableVertexAttribArray(positive_attrib);

	glUseProgram(0);
}
