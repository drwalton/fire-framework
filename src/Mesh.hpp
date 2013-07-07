#ifndef MESH_H
#define MESH_H

#include "Shader.hpp"
#include "Renderable.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>

/* Mesh
 * Supports loading & rendering of meshes loaded via the AssImp library.]
 * In general, a file can contain multiple meshes. Thus, meshes should be 
 * loaded via Mesh::loadFile() which returns a vector of pointers to 
 * the loaded mesh objects.
 * **N.B.** Only the mesh itself (vertices, elements, norms) are loaded. 
 *          Textures, other attributes are ignored. 
 */

class Mesh : public Solid
{
public:
	static std::vector<Mesh*> loadFile(const std::string& filename, 
		LightShader* _shader);
	Mesh(const std::vector<glm::vec4>& v, 
		 const std::vector<glm::vec3>& n,
		 const std::vector<GLushort>& e,
		 LightShader* _shader);
	void render();
	void update(int dTime) {};
private:
	size_t numElems;
	GLuint v_vbo;
	GLuint n_vbo;
	GLuint e_vbo;
	GLuint v_attrib;
	GLuint n_attrib;
	GLuint e_attrib;
};

#endif
