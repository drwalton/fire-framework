#ifndef PARTICLES_HPP
#define PARTICLES_HPP

#include <SOIL.h>
#include "glm.hpp"

#include "Texture.hpp"
#include "Shader.hpp"
#include "Light.hpp"

#include <vector>

/* ParticleSystem
 * An ADT for a renderable object which is a particle system.
 */
template <int maxParticles>
class ParticleSystem : public Renderable
{
public:
	ParticleSystem(ParticleShader* _shader) :shader(_shader) {};
	Shader* getShader() {return (Shader*) shader;};
protected:
	ParticleShader* shader;
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
 */
template <int maxParticles>
class AdvectParticles : public ParticleSystem<maxParticles>
{
public:
	AdvectParticles(ParticleShader* _shader, Texture* _bbTex, Texture* _decayTex);
	AdvectParticles(ParticleShader* _shader, Texture* _bbTex, Texture* _decayTex,
		int avgLifetime, int varLifetime, 
		glm::vec4 initAcn, glm::vec4 initVel,
		int perturbChance, float perturbRadius,
		float baseRadius, float centerForce,
		float bbHeight, float bbWidth,
		bool perturb_on, bool _init_perturb);

	void render();
	virtual void update(int dTime);
protected:
	std::array<glm::vec4, maxParticles> pos;
	int randi(int low, int high);
	float randf(float low, float high);
private:
	const int avgLifetime;
	const int varLifetime;
	const glm::vec4 initAcn;
	const glm::vec4 initVel;
	const int perturbChance; 
	const float perturbRadius;
	const float baseRadius;
	const float centerForce;
	const float bbHeight; //Particle billboard width.
	const float bbWidth;  //Particle billboard height.
	glm::vec3 cameraDir;
	std::array<glm::vec4, maxParticles> vel;
	std::array<glm::vec4, maxParticles> acn;
	std::array<int,       maxParticles> time;
	std::array<int,       maxParticles> lifeTime;
	std::array<float,     maxParticles> decay;
	GLuint pos_vbo;
	GLuint decay_vbo;
	GLuint pos_attrib;
	GLuint decay_attrib;
	bool perturb_on;
	bool init_perturb;

	Texture* bbTex;
	Texture* decayTex;

	void updateParticle(int index, int dTime);
	void spawnParticle(int index);
	void init(Texture* bbTex, Texture* decayTex);
	glm::vec4 perturb(glm::vec4 input);
	glm::vec4 randInitPos();
};

template <int maxParticles>
class AdvectParticlesRandLights : public AdvectParticles<maxParticles>
{
public:
	AdvectParticlesRandLights(int _nLights, ParticleShader* _shader, 
		Texture* _bbTex, Texture* _decayTex);
	AdvectParticlesRandLights(int nLights, ParticleShader* _shader, 
		Texture* _bbTex, Texture* _decayTex,
		int avgLifetime, int varLifetime, 
		glm::vec4 initAcn, glm::vec4 initVel,
		int perturbChance, float perturbRadius,
		float baseRadius, float centerForce,
		float bbHeight, float bbWidth,
		bool perturb_on, bool _init_perturb);
	const int nLights;
	std::vector<PointLight*> lights;
	void onAdd();
	void onRemove();
	void update(int dTime);
private:
	void updateLights();
};

#include "Particles.cpp"

#endif
