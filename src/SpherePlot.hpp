#ifndef SPHEREPLOT_HPP
#define SPHEREPLOT_HPP

#include "SH.hpp"
#include "Renderable.hpp"
#include <GL/glew.h>

struct SpherePlotVertex
{
	glm::vec4 pos;
	glm::vec3 norm;
	float pad;
	float positive;
};

struct SpherePlotMesh
{
	std::vector<SpherePlotVertex> v;
	std::vector<GLushort>         e;
};

struct SphereSample
{
	float theta;
	float phi;
	float val;
};

/* SpherePlot
 * A Renderable containing a 3D plot of 
 * a function $S_2 \rightarrow \mathbb{R}$.
 */
class SpherePlot : public Renderable
{
public:
	template <typename Func>
	SpherePlot(Func f, unsigned sqrtNSamples, Shader* shader);
	void render();
	void update(int dTime) {};
	Shader* getShader() {return shader;};
private:
	template <typename Func>
	std::vector<SphereSample> takeSamples(
		Func f, unsigned sqrtNSamples);
	SpherePlotMesh genMesh(
		const std::vector<SphereSample>& samples);
	void uploadMeshToGPU(const SpherePlotMesh& mesh);

	Shader* shader;

	GLsizei numElems;

	GLuint v_vbo;
	GLuint e_vbo;
	GLuint pos_attrib;
	GLuint norm_attrib;
	GLuint positive_attrib;
};

template <typename Func>
SpherePlot::SpherePlot(Func f, unsigned sqrtNSamples, Shader* shader)
	:Renderable(false), shader(shader)
{
	auto samples = takeSamples(f, sqrtNSamples);
	auto mesh = genMesh(samples);
	uploadMeshToGPU(mesh);
}

template <typename Func>
std::vector<SphereSample> SpherePlot::takeSamples(
	Func f, unsigned sqrtNSamples)
{
	std::vector<SphereSample> samples;

	double sqrWidth = 1 / (double) sqrtNSamples;
	double u, v, theta, phi;	 

	for(int i = 0; i < sqrtNSamples + 1; ++i)
		for(int j = 0; j < sqrtNSamples; ++j)
		{
			u = (i * sqrWidth);
			v = (j * sqrWidth);
			theta = acos((2 * u) - 1);
			phi = 2 * PI_d * v;

			SphereSample s;
			s.theta = theta; s.phi = phi;
			s.val = f(theta, phi);

			samples.push_back(s);
		}

	return samples;
}

#endif
