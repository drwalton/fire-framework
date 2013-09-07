#ifndef AOMESH_HPP
#define AOMESH_HPP

#include "Renderable.hpp"
#include "Shader.hpp"

#include <GL/glut.h>

#include <string>
#include <vector>

struct MeshData;
class Texture;

struct AOMeshVertex
{
	glm::vec4  v; //Position
	glm::vec3  n; //Normal
	glm::vec3  bn; //Bent normal
	glm::vec2  t; //Tex coord
};

/* AOMesh
 * Class representing an object rendered using
 * AO and Blinn-Phong shading. 
 * Intended to be used by first calling bake() to
 * create a pre-baked file, and then loading this
 * via the constructor to create AOMesh objects.
 */
class AOMesh : public Renderable
{
public:
	AOMesh(
		const std::string& bakedFilename,
		LightShader* shader);

	static void bake(
		const std::string& coarseMeshFilename,
		const std::string& fineMeshFilename,
		const std::string& bakedFilename,
		const std::string& ambTex,
		const std::string& diffTex,
		const std::string& specTex,
		float specExp,
		int sqrtNSamples);

	static void writePrebakedFile(
		const std::vector<AOMeshVertex>& mesh,
		const std::vector<GLushort>& elems,
		const std::string& ambTex,
		const std::string& diffTex,
		const std::string& specTex,
		float specExp,
	 	const std::string& filename);

	void render();
	void update(int dTime) {};
	Shader* getShader() {return shader;};
private:
	void readPrebakedFile(
		std::vector<AOMeshVertex>& mesh,
		std::vector<GLushort>& elems,
	 	const std::string& filename);
	void init(
		const std::vector<AOMeshVertex>& mesh,
		const std::vector<GLushort>& elems);
	static void renderOcclToImage(
		const std::vector<float>& vertOccl,
		const std::string& ambIm,
		const std::string& bakedIm,
		const MeshData& data);

	LightShader* shader;
	size_t numElems;

	Texture* ambTex;
	Texture* diffTex;
	Texture* specTex;
	float specExp;

	GLuint vao;
	GLuint v_vbo;
	GLuint e_vbo;
	GLuint v_attrib;
	GLuint n_attrib;
	GLuint bn_attrib;
	GLuint t_attrib;
};

#endif
