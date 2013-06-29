#include "Renderable.hpp"

Renderable::Renderable(Shader* _render)
	:renderShader(_render),
	scene(nullptr),
	modelToWorld(glm::mat4(1.0))
{
	
}

template<int nVertices>
ArrSolid<nVertices>::ArrSolid(LightShader* _render,
	const std::array<glm::vec4, nVertices>& _v,
	const std::array<glm::vec3, nVertices>& _n)
	:Solid(_render), v(_v), n(_n)
{
	glGenBuffers(1, &v_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, v_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(v), &v, GL_STATIC_DRAW);
	glGenBuffers(1, &n_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, n_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(n), &n, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	v_attrib = renderShader->getAttribLoc("vPosition");
	n_attrib = renderShader->getAttribLoc("vNorm");
}

template<int nVertices>
void ArrSolid<nVertices>::render()
{

	if(!scene) return;
	renderShader->setModelToWorld(modelToWorld);

	renderShader->use();
	glEnableVertexAttribArray(v_attrib);
	glEnableVertexAttribArray(n_attrib);
	glBindBuffer(GL_ARRAY_BUFFER, v_vbo);
	glVertexAttribPointer(v_attrib, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, n_vbo);
	glVertexAttribPointer(n_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDrawArrays(GL_TRIANGLES, 0, nVertices);

	glDisableVertexAttribArray(v_attrib);
	glDisableVertexAttribArray(n_attrib);
	glUseProgram(0);
}

ArrSolid<36>* Solid::Cube(LightShader* _render)
{
	std::array<glm::vec4, 36> v;
	std::array<glm::vec3, 36> n;
	//Top
	v[0 ] = glm::vec4( 0.5,  0.5, -0.5, 1.0);
	v[1 ] = glm::vec4(-0.5,  0.5, -0.5, 1.0);
	v[2 ] = glm::vec4(-0.5,  0.5,  0.5, 1.0);

	v[3 ] = glm::vec4( 0.5,  0.5,  0.5, 1.0);
	v[4 ] = glm::vec4( 0.5,  0.5, -0.5, 1.0);
	v[5 ] = glm::vec4(-0.5,  0.5,  0.5, 1.0);

	//Front
	v[6 ] = glm::vec4( 0.5,  0.5,  0.5, 1.0);
	v[7 ] = glm::vec4(-0.5,  0.5,  0.5, 1.0);
	v[8 ] = glm::vec4(-0.5, -0.5,  0.5, 1.0);

	v[9 ] = glm::vec4( 0.5, -0.5,  0.5, 1.0);
	v[10] = glm::vec4( 0.5,  0.5,  0.5, 1.0);
	v[11] = glm::vec4(-0.5, -0.5,  0.5, 1.0);

	//Bottom
	v[12] = glm::vec4( 0.5, -0.5,  0.5, 1.0);
	v[13] = glm::vec4(-0.5, -0.5,  0.5, 1.0);
	v[14] = glm::vec4(-0.5, -0.5, -0.5, 1.0);

	v[15] = glm::vec4( 0.5, -0.5, -0.5, 1.0);
	v[16] = glm::vec4( 0.5, -0.5,  0.5, 1.0);
	v[17] = glm::vec4(-0.5, -0.5, -0.5, 1.0);

	//Back
	v[18] = glm::vec4( 0.5, -0.5, -0.5, 1.0);
	v[19] = glm::vec4(-0.5, -0.5, -0.5, 1.0);
	v[20] = glm::vec4(-0.5,  0.5, -0.5, 1.0);

	v[21] = glm::vec4( 0.5,  0.5, -0.5, 1.0);
	v[22] = glm::vec4( 0.5, -0.5, -0.5, 1.0);
	v[23] = glm::vec4(-0.5,  0.5, -0.5, 1.0);

	//Left
	v[24] = glm::vec4(-0.5,  0.5,  0.5, 1.0);
	v[25] = glm::vec4(-0.5,  0.5, -0.5, 1.0);
	v[26] = glm::vec4(-0.5, -0.5, -0.5, 1.0);

	v[27] = glm::vec4(-0.5, -0.5,  0.5, 1.0);
	v[28] = glm::vec4(-0.5,  0.5,  0.5, 1.0);
	v[29] = glm::vec4(-0.5, -0.5, -0.5, 1.0);

	//Right
	v[30] = glm::vec4( 0.5,  0.5, -0.5, 1.0);
	v[31] = glm::vec4( 0.5,  0.5,  0.5, 1.0);
	v[32] = glm::vec4( 0.5, -0.5,  0.5, 1.0);

	v[33] = glm::vec4( 0.5, -0.5, -0.5, 1.0);
	v[34] = glm::vec4( 0.5,  0.5, -0.5, 1.0);
	v[35] = glm::vec4( 0.5, -0.5,  0.5, 1.0);


	for(int i = 0; i < 6; ++i)
	{
		n[i   ] = glm::vec3( 0.0,  1.0,  0.0); //Top
		n[i+6 ] = glm::vec3( 0.0,  0.0,  1.0); //Front
		n[i+12] = glm::vec3( 0.0, -1.0,  0.0); //Bottom
		n[i+18] = glm::vec3( 0.0,  0.0, -1.0); //Back
		n[i+24] = glm::vec3( 0.0, -1.0,  0.0); //Left
		n[i+30] = glm::vec3( 0.0,  1.0,  0.0); //Right
	}

	ArrSolid<36>* cube = new ArrSolid<36>(_render, v, n);
	return cube;
}
