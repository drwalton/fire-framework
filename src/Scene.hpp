#ifndef SCENE_H
#define SCENE_H

#include "Renderable.hpp"
#include "Particles.hpp"
#include "Light.hpp"
#include "Shader.hpp"
#include "Camera.hpp"

#include <glm.hpp>
#include <glut.h>
#include <SOIL.h>

#include <vector>
#include <set>

const float PI = 3.141592653589793238462f;

/* Scene
 * The Scene object handles all Element objects added to it, and renders them appropriately.
 * Destroying a Scene also calls the destructors of added objects.
 */
class Scene
{
public:
	Scene();
	~Scene();
	void render();
	void update(int dTime);

	void add(Renderable* r);
	void remove(Renderable* r);

	void add(DirLight* d);
	void updateLight(DirLight* d);
	void remove(DirLight* d);

	void add(PointLight* p);
	void updateLight(PointLight* p);
	void remove(PointLight* p);

	void setAmbLight(float _ambLight);
	
	static const int maxDirLights = 10;
	GLuint* getDirLightOn() {return dirLightOn;};
	glm::vec3* getDirLightDir();

	static const int maxPointLights = 10;
	GLuint* getPointLightOn() {return pointLightOn;};
	glm::vec4* getPointLightPos();
	
	Camera* camera;
private:
	float ambLight;
	GLuint ambLight_u;

	std::set<Renderable*> renderables;

	std::set<Shader*> shaders;

	int nDirLights;
	DirLight* dirLights[maxDirLights];
	GLuint dirLightOn[maxDirLights];
	glm::vec3 dirLightDir[maxDirLights];
	float dirIntensity[maxDirLights];
	void updateDirLights();
	
	int nPointLights;
	PointLight* pointLights[maxPointLights];
	GLuint pointLightOn[maxPointLights];
	glm::vec4 pointLightPos[maxPointLights];
	float pointIntensity[maxPointLights];
	void updatePointLights();

	void updateCamera();
};

#endif
