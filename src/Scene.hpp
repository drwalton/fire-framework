#ifndef SCENE_H
#define SCENE_H

#include "Renderable.hpp"
#include "Particles.hpp"
#include "Light.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"

#include <glm.hpp>
#include <glut.h>
#include <SOIL.h>

#include <vector>
#include <set>

class DirLight;
class PointLight;

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
	/* render() renders all renderables added to the scene.
	 * Opaque objects are rendered first, transparent second.
	 * Within these categories, renderables are rendered in the order added.
	 */
	void render();
	void update(int dTime);

	/* add() and remove() functions return a pointer to the element added/removed.
	 * e.g. Renderable* p = scene.add(new AdvectParticles(s, t1, t2));
	 * If the element could not be added/removed (e.g. maxPointLights is exceeded),
	 * nullptr is returned.
	 */
	Renderable* add(Renderable* r);
	Renderable* remove(Renderable* r);

	DirLight* add(DirLight* d);
	DirLight* updateLight(DirLight* d);
	DirLight* remove(DirLight* d);

	PointLight* add(PointLight* p);
	PointLight* updateLight(PointLight* p);
	PointLight* remove(PointLight* p);

	void setAmbLight(float _ambLight);
	
	static const int maxDirLights = 50;
	GLuint* getDirLightOn() {return dirLightOn;};
	glm::vec3* getDirLightDir();

	static const int maxPointLights = 50;
	GLuint* getPointLightOn() {return pointLightOn;};
	glm::vec4* getPointLightPos();
	
	static const int maxSHLights = 10;
	static const int nSHBands = 5;
	static const int sqrtSHSamples = 100;

	Camera* camera;
private:
	float ambLight;
	GLuint ambLight_u;

	std::set<Renderable*> opaque;
	std::set<Renderable*> translucent;

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
