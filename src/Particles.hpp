#ifndef PARTICLES_HPP
#define PARTICLES_HPP

#include "Renderable.hpp"
#include "Shader.hpp"
#include "GC.hpp"

#include <GL/glew.h>
#include <glm.hpp>

#include <vector>
#include <array>

class Texture;
class PhongLight;
class SHLight;
class ParticleShader;

/* ParticleSystem
 * An ADT for a renderable object which is a particle system.
 */
class ParticleSystem : public Renderable
{
public:
	ParticleSystem(int _maxParticles, ParticleShader* _shader, float alpha = 0.5f) 
		:Renderable(true), maxParticles(_maxParticles), shader(_shader), alpha(alpha) {};
	ParticleShader* getParticleShader() {return shader;};
	Shader* getShader() {return static_cast<Shader*>(shader);};
	virtual void setShader(ParticleShader* shader) = 0;
protected:
	int maxParticles;
	float alpha;
	ParticleShader* shader;
};

struct AdvectParticle
{
	glm::vec4 pos;
	GLfloat decay;
	GLfloat randTex;
};


/* AdvectParticles
 * A ParticleSystem consisting of MaxParticles particles, which behave as follows:
 * #1: Particles spawn at a random point in a disk of radius baseRadius, around (0,0,0) in model space.
 * #2: Particles have an initial acceleration of initAcn, an intial velocity of initVel.
 * #3: On each frame, particles have a 1/perturbChance probability of being perturbed.
 *  	In this case, the particle is given a random force in the disk of radius perturbRadius in the x-z plane.
 * #4: A centering force of magnitude centerForce pulls the particles towards the y-axis in model space at all times.
 * #5: Particles live for a random lifetime in [avgLifetime - varLifetime, avgLifetime + varLifetime] (units ms). 
 * 		Upon death, a new particle is spawned (so MaxParticles particles are present at all times).
 * Particles are rendered as billboards of height bbHeight, width bbWidth. 
 * The bbTex is applied to each particle billboard. The colour of the billboard is set by a point along
 *  decayTex determined by the particle's remaining lifetime.
 * **Note** that scrollTexParticles.glsl uses bbTex in a different way. See the shader source for more details.
 * The additive property determines whether additive or subtractive alpha 
 *   blending is used.
 */
class AdvectParticles : public ParticleSystem
{
public:
	AdvectParticles(int maxParticles, ParticleShader* shader,
		Texture* bbTex, Texture* decayTex, bool texScrolls = true, bool additive = true);
	AdvectParticles(int maxParticles, ParticleShader* shader,
		Texture* bbTex, Texture* decayTex,
		int avgLifetime, int varLifetime, 
		glm::vec4 initAcn, glm::vec4 initVel,
		int avgPerturbTime, int varPerturbTime, float perturbRadius,
		float baseRadius, float centerForce,
		float bbHeight, float bbWidth,
		bool perturbOn, bool initPerturb, bool texScrolls = true, bool additive = true);

	void render();
	virtual void update(int dTime);
	virtual void setShader(ParticleShader* shader);
	void setExtForce(const glm::vec3& extForce);
protected:
	bool additive;
	std::vector<AdvectParticle> particles;
	int randi(int low, int high);
	float randf(float low, float high);

	GLuint particles_vbo;
	GLuint pos_attrib;
	GLuint decay_attrib;
	GLuint randTex_attrib;

	Texture* bbTex;
	Texture* decayTex;
private:
	const int avgLifetime;
	const int varLifetime;
	const glm::vec4 initAcn;
	const glm::vec4 initVel;
	const int avgPerturbTime;
	const int varPerturbTime;
	const float perturbRadius;
	const float baseRadius;
	const float centerForce;
	const float bbHeight; //Particle billboard width.
	const float bbWidth;  //Particle billboard height.
	glm::vec3 cameraDir;

	std::vector<glm::vec4> vel;
	std::vector<glm::vec4> acn;
	glm::vec4 extForce; //External force applied to all particles.
	std::vector<int> time;
	std::vector<int> lifeTime;
	std::vector<int> perturbCounter;
	std::vector<int> perturbTime;

	bool perturbOn;
	bool initPerturb;

	void updateParticle(int index, int dTime);
	void spawnParticle(int index);
	void init(Texture* bbTex, Texture* decayTex, bool texScrolls);

	glm::vec4 perturb(glm::vec4 input);
	glm::vec4 randInitPos();
};

/* AdvectParticlesLights
 * ADT for a derived class of AdvectParticles owning a number of light
 * sources
 */
class AdvectParticlesLights : public AdvectParticles
{
public:
	AdvectParticlesLights(int _maxParticles,
		int _nLights, ParticleShader* _shader, 
		Texture* _bbTex, Texture* _decayTex);
	AdvectParticlesLights(int _maxParticles, 
		int nLights, ParticleShader* _shader, 
		Texture* _bbTex, Texture* _decayTex,
		int avgLifetime, int varLifetime, 
		glm::vec4 initAcn, glm::vec4 initVel,
		int avgPerturbTime, int varPerturbTime, float perturbRadius,
		float baseRadius, float centerForce,
		float bbHeight, float bbWidth,
		bool perturb_on, bool _init_perturb);
	const int nLights;
	std::vector<PhongLight*> lights;
	void onAdd();
	void onRemove();
	void update(int dTime);
protected:
	virtual void updateLights() = 0;
};

/* AdvectParticlesRandLights
 * Places nLights lights at the location of nLights particles.
 * The particles are initially randomly selected, and the lights
 * follow them, hopping to new randomly selected particles whenever
 * an interval elapses.
 * Special cases: 
 *     * interval == 0 : Lights hop every frame.
 *     * interval == -1: Lights never hop.
 */
class AdvectParticlesRandLights : public AdvectParticlesLights
{
public:
	AdvectParticlesRandLights(
		int _maxParticles, int _nLights,
		int _interval, ParticleShader* _shader, 
		Texture* _bbTex, Texture* _decayTex);
	AdvectParticlesRandLights(
		int _maxParticles, int _nLights,
		int _interval, ParticleShader* _shader, 
		Texture* _bbTex, Texture* _decayTex,
		int avgLifetime, int varLifetime, 
		glm::vec4 initAcn, glm::vec4 initVel,
		int avgPerturbTime, int varPerturbTime, float perturbRadius,
		float baseRadius, float centerForce,
		float bbHeight, float bbWidth,
		bool perturb_on, bool _init_perturb);
protected:
	void updateLights();
private:
	int counter;
	const int interval;
	void init();
	void randomizeLights();
	std::vector<int> lightIndices;
};

/* AdvectParticlesCentroidLights
 * Similar approach to AdvectParticlesRandLights, but each light is placed at the 
 * centroid of "clumpSize" particles.
 */
class AdvectParticlesCentroidLights : public AdvectParticlesLights
{
public:
	AdvectParticlesCentroidLights(
		int _maxParticles, int _nLights, int _clumpSize,
		int _interval, ParticleShader* _shader, 
		Texture* _bbTex, Texture* _decayTex);
	AdvectParticlesCentroidLights(
		int _maxParticles, int _nLights, int _clumpSize,
		int _interval, ParticleShader* _shader, 
		Texture* _bbTex, Texture* _decayTex,
		int avgLifetime, int varLifetime, 
		glm::vec4 initAcn, glm::vec4 initVel,
		int avgPerturbTime, int varPerturbTime, float perturbRadius,
		float baseRadius, float centerForce,
		float bbHeight, float bbWidth,
		bool perturb_on, bool _init_perturb);
	const int clumpSize;
protected:
	void updateLights();
private:
	int counter;
	const int interval;
	void init();
	void randomizeClumps();
	std::vector<std::vector<int>> clumps;
	glm::vec4 getParticleCentroid(const std::vector<int>& clump);
};

/* AdvectParticlesSHLights
 * ADT for a derived class of AdvectParticles owning a number of SH light
 * sources
 */
class AdvectParticlesSHLights : public AdvectParticles
{
public:
	AdvectParticlesSHLights(
		Renderable* targetObj,
		int _maxParticles,
		int _nLights, ParticleShader* _shader, 
		Texture* _bbTex, Texture* _decayTex);
	AdvectParticlesSHLights(
		Renderable* targetObj,
		int _maxParticles, 
		int nLights, ParticleShader* _shader, 
		Texture* _bbTex, Texture* _decayTex,
		int avgLifetime, int varLifetime, 
		glm::vec4 initAcn, glm::vec4 initVel,
		int avgPerturbTime, int varPerturbTime, float perturbRadius,
		float baseRadius, float centerForce,
		float bbHeight, float bbWidth,
		bool perturb_on, bool _init_perturb);
	const int nLights;
	std::vector<SHLight*> lights;
	void onAdd();
	void onRemove();
	void update(int dTime);
protected:
	virtual void updateLights() = 0;
	void makeLights();
	Renderable* targetObj;
};

/* AdvectParticlesRandSHLights
 * Copy of AdvectParticlesRandLights designed to manipulate SH lights.
 */
class AdvectParticlesRandSHLights : public AdvectParticlesSHLights
{
public:
	AdvectParticlesRandSHLights(
		Renderable* targetObj,
		int _maxParticles, int _nLights,
		int _interval, ParticleShader* _shader, 
		Texture* _bbTex, Texture* _decayTex);
	AdvectParticlesRandSHLights(
		Renderable* targetObj,
		int _maxParticles, int _nLights,
		int _interval, ParticleShader* _shader, 
		Texture* _bbTex, Texture* _decayTex,
		int avgLifetime, int varLifetime, 
		glm::vec4 initAcn, glm::vec4 initVel,
		int avgPerturbTime, int varPerturbTime, float perturbRadius,
		float baseRadius, float centerForce,
		float bbHeight, float bbWidth,
		bool perturb_on, bool _init_perturb);
protected:
	void updateLights();
private:
	int counter;
	const int interval;
	void init();
	void randomizeLights();
	std::vector<int> lightIndices;
};

/* AdvectParticlesCentroidSHLights
 * Copy of AdvectParticlesCentroidLights for SH lights.
 */
class AdvectParticlesCentroidSHLights : public AdvectParticlesSHLights
{
public:
	AdvectParticlesCentroidSHLights(
		Renderable* targetObj,
		int _maxParticles, int _nLights, int _clumpSize,
		int _interval, ParticleShader* _shader, 
		Texture* _bbTex, Texture* _decayTex);
	AdvectParticlesCentroidSHLights(
		Renderable* targetObj,
		int _maxParticles, int _nLights, int _clumpSize,
		int _interval, ParticleShader* _shader, 
		Texture* _bbTex, Texture* _decayTex,
		int avgLifetime, int varLifetime, 
		glm::vec4 initAcn, glm::vec4 initVel,
		int avgPerturbTime, int varPerturbTime, float perturbRadius,
		float baseRadius, float centerForce,
		float bbHeight, float bbWidth,
		bool perturb_on, bool _init_perturb);
	const int clumpSize;
protected:
	void updateLights();
private:
	int counter;
	const int interval;
	void init();
	void randomizeClumps();
	std::vector<std::vector<int>> clumps;
	glm::vec4 getParticleCentroid(const std::vector<int>& clump);
};

/* AdvectParticlesCentroidSHLights
 * Copy of AdvectParticlesCentroidLights for SH lights.
 */
class AdvectParticlesSHCubemap : public AdvectParticles
{
public:
	AdvectParticlesSHCubemap(
		Renderable* targetObj,
		int _maxParticles, ParticleShader* _shader, 
		Texture* _bbTex, Texture* _decayTex);
	AdvectParticlesSHCubemap(
		Renderable* targetObj,
		int _maxParticles, ParticleShader* _shader, 
		Texture* _bbTex, Texture* _decayTex,
		int avgLifetime, int varLifetime, 
		glm::vec4 initAcn, glm::vec4 initVel,
		int avgPerturbTime, int varPerturbTime, float perturbRadius,
		float baseRadius, float centerForce,
		float bbHeight, float bbWidth,
		bool perturb_on, bool _init_perturb);
	void update(int dTime);
	void onAdd();
private:
	Renderable* targetObj;
	CubemapShader* cubemapShader;
	SHLight* light;
	void init();
	void renderCubemap();
	void updateLight();
	std::array<std::array<glm::vec4, GC::cubemapPixels>, 6> cubemap;
	glm::vec3 cubemapLookup(float theta, float phi);
	int findFace(glm::vec3 dir);
	glm::mat4 getRotation(int face);
	GLuint renderbuffer;
	GLuint framebuffer;
};

#endif
