#ifndef PARTICLES_HPP
#define PARTICLES_HPP

#include <SOIL.h>
#include "glm.hpp"

#include "Texture.hpp"
#include "Shader.hpp"

template <int MaxParticles>
class ParticleSystem : public Renderable
{
public:
	ParticleSystem(Shader* _renderShader) :Renderable(_renderShader) {};
};

template <int MaxParticles>
class AdvectParticles : public ParticleSystem<MaxParticles>
{
public:
	AdvectParticles(Shader* _renderShader, Texture* _bbTex, Texture* _decayTex);
	AdvectParticles(Shader* _renderShader, Texture* _bbTex, Texture* _decayTex,
		int avgLifetime, int varLifetime, 
		glm::vec4 initAcn, glm::vec4 initVel,
		int perturbChance, float perturbRadius,
		float baseRadius, float centerForce,
		float bbHeight, float bbWidth,
		bool perturb_on, bool _init_perturb);

	void render();
	void update(int dTime);
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
	glm::vec3 cameraPos;
	std::array<glm::vec4, MaxParticles> pos;
	std::array<glm::vec4, MaxParticles> vel;
	std::array<glm::vec4, MaxParticles> acn;
	std::array<int,       MaxParticles> time;
	std::array<int,       MaxParticles> lifeTime;
	std::array<float,     MaxParticles> decay;
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
	int randi(int low, int high);
	float randf(float low, float high);
	glm::vec4 randInitPos();
};

#endif
