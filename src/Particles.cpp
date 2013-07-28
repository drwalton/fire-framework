#include "Particles.hpp"

AdvectParticles::AdvectParticles(int _maxParticles,
	ParticleShader* _shader, 
	Texture* _bbTex, Texture* _decayTex)
	:ParticleSystem(_maxParticles, _shader),
	 bbTex(_bbTex), decayTex(_decayTex),
	 avgLifetime(1500), varLifetime(200),
	 perturbChance(10),
	 initAcn(glm::vec4(0.0, 0.000001, 0.0, 0.0)),
	 initVel(glm::vec4(0.0, 0.0, 0.0, 0.0)),
	 perturbRadius(0.0005f),
	 centerForce(0.00001f),
	 baseRadius(0.2f),
	 bbHeight(0.2f), bbWidth(0.2f),
	 perturb_on(true), init_perturb(false),
	 cameraDir(glm::vec3(0.0, 0.0, -1.0)),
	 vel(std::vector<glm::vec4>(maxParticles, glm::vec4(0.0f))),
	 acn(std::vector<glm::vec4>(maxParticles, glm::vec4(0.0f))),
	 time(std::vector<int>(maxParticles, 0)),
	 lifeTime(std::vector<int>(maxParticles, 0))
{init(bbTex, decayTex);}

AdvectParticles::AdvectParticles(int _maxParticles,
	ParticleShader* _shader, 
	Texture* _bbTex, Texture* _decayTex,
	int _avgLifetime, int _varLifetime, 
	glm::vec4 _initAcn, glm::vec4 _initVel,
	int _perturbChance, float _perturbRadius,
	float _baseRadius, float _centerForce,
	float _bbHeight, float _bbWidth,
	bool _perturb_on, bool _init_perturb)
	:ParticleSystem(_maxParticles, _shader),
     bbTex(_bbTex), decayTex(_decayTex),
	 avgLifetime(_avgLifetime), varLifetime(_varLifetime),
	 perturbChance(_perturbChance),
	 initAcn(_initAcn),
	 initVel(_initVel),
	 perturbRadius(_perturbRadius),
	 centerForce(_centerForce),
	 baseRadius(_baseRadius),
	 bbHeight(_bbHeight), bbWidth(_bbWidth),
	 perturb_on(_perturb_on), init_perturb(_init_perturb),
	 cameraDir(glm::vec3(0.0, 0.0, -1.0)),
	 vel(std::vector<glm::vec4>(maxParticles, glm::vec4(0.0f))),
	 acn(std::vector<glm::vec4>(maxParticles, glm::vec4(0.0f))),
	 time(std::vector<int>(maxParticles, 0)),
	 lifeTime(std::vector<int>(maxParticles, 0))
{init(bbTex, decayTex);}

void AdvectParticles::init(Texture* bbTex, Texture* decayTex)
{

	shader->use();

	// Set up particles.
	for(int i = 0; i < maxParticles; ++i)
	{
		AdvectParticle p;
		p.pos = randInitPos();
		p.decay = 0.0f;
		particles.push_back(p);

		time.push_back(0);
		lifeTime.push_back(avgLifetime + randi(-varLifetime, +varLifetime));
		acn.push_back(initAcn);
		
		if(init_perturb) vel.push_back(perturb(vel[i]));
		else vel.push_back(initVel);
	}

	glUseProgram(0);

	shader->setBBTexUnit(bbTex->getTexUnit());
	shader->setDecayTexUnit(decayTex->getTexUnit());

	// Set up vertex buffer objects.
	glGenBuffers(1, &particles_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, particles_vbo);
	glBufferData(GL_ARRAY_BUFFER, particles.size()*sizeof(AdvectParticle),
		particles.data(), GL_DYNAMIC_DRAW);

	// Set up uniforms.
	shader->setBBWidth(bbWidth);
	shader->setBBHeight(bbHeight);

	pos_attrib = shader->getAttribLoc("vPos");
	decay_attrib = shader->getAttribLoc("vDecay");
}

void AdvectParticles::render()
{
	if(!scene) return;
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shader->setModelToWorld(modelToWorld);

	shader->setBBTexUnit(bbTex->getTexUnit());
	shader->setDecayTexUnit(decayTex->getTexUnit());

	shader->use();

	glBindBuffer(GL_ARRAY_BUFFER, particles_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particles.size()*sizeof(AdvectParticle),
		particles.data());

	glEnableVertexAttribArray(pos_attrib);
	glEnableVertexAttribArray(decay_attrib);

	glBindVertexBuffer(0, particles_vbo, 0, sizeof(AdvectParticle));
	glVertexAttribFormat(pos_attrib, 4, GL_FLOAT, GL_FALSE,
		offsetof(AdvectParticle, pos));
	glVertexAttribBinding(pos_attrib, 0);
	glVertexAttribFormat(decay_attrib, 1, GL_FLOAT, GL_FALSE,
		offsetof(AdvectParticle, decay));
	glVertexAttribBinding(decay_attrib, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glDrawArrays(GL_POINTS, 0, particles.size());

	glDisableVertexAttribArray(pos_attrib);
	glDisableVertexAttribArray(decay_attrib);

	glUseProgram(0);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

void AdvectParticles::update(int dTime)
{
	for(int i = 0; i < maxParticles; ++i)
		updateParticle(i, dTime);
}

void AdvectParticles::updateParticle(int index, int dTime)
{
	time[index] += dTime;
	if(time[index] > lifeTime[index]) spawnParticle(index);
	particles[index].decay = ((float) time[index]) / ((float) lifeTime[index]);

	if(time[index] % perturbChance == 1 && perturb_on)
		vel[index] = perturb(vel[index]);

	vel[index] += (float) dTime *
		(acn[index] + (glm::vec4(
			-particles[index].pos.x,
			0.0,
			-particles[index].pos.z,
			0.0)
			* centerForce));
	particles[index].pos += (float) dTime * vel[index];
}

void AdvectParticles::spawnParticle(int index)
{
	time[index] = 0;
	lifeTime[index] = avgLifetime + randi(-varLifetime, +varLifetime);
	particles[index].decay = 0.0;
	acn[index] = initAcn;
	vel[index] = initVel;
	particles[index].pos = randInitPos();
}

glm::vec4 AdvectParticles::randInitPos()
{
	float theta = randf(0.0f, 2.0f * PI);
	float radius = randf(0.0f, baseRadius);
	return glm::vec4(radius*cos(theta), 0.0, radius*sin(theta), 1.0);
}

glm::vec4 AdvectParticles::perturb(glm::vec4 input)
{
	float theta = randf(0.0f, 2.0f * PI);
	float radius = randf(0.0f, perturbRadius);
	return input + glm::vec4(radius * cos(theta), 0.0, radius * sin(theta), 0.0);
}

float AdvectParticles::randf(float low, float high)
{
	float r = (float) rand() / (float) RAND_MAX;
	return low + ((high - low) * r);
}

int AdvectParticles::randi(int low, int high)
{
	int r = rand() % (high - low);
	return r + low;
}

AdvectParticlesLights::AdvectParticlesLights(int _maxParticles, int _nLights, 
	ParticleShader* _shader, Texture* _bbTex, Texture* _decayTex)
	:AdvectParticles(_maxParticles, _shader, _bbTex, _decayTex),
	 nLights(_nLights)
{
	// Set up vector of lights.
	for(int i = 0; i < nLights; ++i)
	{
		lights.push_back(new PhongLight(getOrigin()));
	}
}

AdvectParticlesLights::AdvectParticlesLights(
	int _maxParticles,
	int _nLights, ParticleShader* _shader, 
	Texture* _bbTex, Texture* _decayTex,
	int avgLifetime, int varLifetime, 
	glm::vec4 initAcn, glm::vec4 initVel,
	int perturbChance, float perturbRadius,
	float baseRadius, float centerForce,
	float bbHeight, float bbWidth,
	bool perturb_on, bool _init_perturb)
	:AdvectParticles(_maxParticles, 
	 _shader, _bbTex, _decayTex,
	 avgLifetime, varLifetime, 
	 initAcn, initVel,
	 perturbChance, perturbRadius,
	 baseRadius, centerForce,
	 bbHeight, bbWidth,
	 perturb_on, _init_perturb),
	 nLights(_nLights)
{
	// Set up vector of lights.
	for(int i = 0; i < nLights; ++i)
	{
		lights.push_back(new PhongLight(getOrigin()));
	}
}

void AdvectParticlesLights::onAdd()
{
	PhongLight* p;
	// Add lights
	for(auto l = lights.begin(); l != lights.end(); ++l)
	{
		p = scene->add(*l);
		if(p == nullptr) //Light not added correctly (too many lights?).
		{
			std::cout << "Warning: Not all particle lights could be added.\n";
			break;
		}
	}
}

void AdvectParticlesLights::onRemove()
{
	for(auto l = lights.begin(); l != lights.end(); ++l)
		scene->remove(*l);
}

void AdvectParticlesLights::update(int dTime)
{
	AdvectParticles::update(dTime);
	updateLights();
}

AdvectParticlesRandLights::AdvectParticlesRandLights(
	int _maxParticles, int _nLights,
	int _interval, ParticleShader* _shader, 
	Texture* _bbTex, Texture* _decayTex)
	:AdvectParticlesLights(_maxParticles,
		_nLights, _shader, 
		_bbTex, _decayTex),
	interval(_interval), counter(0), 
	lightIndices(std::vector<int>(nLights, 0)) 
{randomizeLights();}

AdvectParticlesRandLights::AdvectParticlesRandLights(
	int _maxParticles, int _nLights,
	int _interval, ParticleShader* _shader, 
	Texture* _bbTex, Texture* _decayTex,
	int avgLifetime, int varLifetime, 
	glm::vec4 initAcn, glm::vec4 initVel,
	int perturbChance, float perturbRadius,
	float baseRadius, float centerForce,
	float bbHeight, float bbWidth,
	bool perturb_on, bool _init_perturb)
	:AdvectParticlesLights(_maxParticles,
	_nLights, _shader, 
	_bbTex, _decayTex,
	avgLifetime, varLifetime, 
	initAcn, initVel,
	perturbChance, perturbRadius,
	baseRadius, centerForce,
	bbHeight, bbWidth,
	perturb_on, _init_perturb),
	interval(_interval), counter(0), 
	lightIndices(std::vector<int>(nLights, 0))
{randomizeLights();}

void AdvectParticlesRandLights::updateLights()
{
	if(interval == 0)
		randomizeLights();
	else if(interval > 0)
	{
		counter += glutGet(GLUT_ELAPSED_TIME);
		if(counter > interval)
		{
			randomizeLights();
			counter = 0;
		}
	}

	for(int i = 0; i < nLights; ++i)
	{
		lights[i]->setPos(modelToWorld * particles[i].pos);
	}
}

void AdvectParticlesRandLights::randomizeLights()
{
	//Move each light to the location of a randomly selected particle.
	for(auto i = lightIndices.begin(); i != lightIndices.end(); ++i)
	{
		int randParticleIndex = randi(0, maxParticles);
		*i = randParticleIndex;
	}
}

AdvectParticlesCentroidLights::AdvectParticlesCentroidLights(
	int _maxParticles,
	int _nLights, int _clumpSize,
	int _interval, ParticleShader* _shader, 
	Texture* _bbTex, Texture* _decayTex)
	:AdvectParticlesLights(_maxParticles, 
		_nLights, _shader, 
		_bbTex, _decayTex),
	 interval(_interval), counter(0),
	 clumpSize(_clumpSize)
{
	init();
}

AdvectParticlesCentroidLights::AdvectParticlesCentroidLights(
	int _maxParticles,
	int _nLights, int _clumpSize,
	int _interval, ParticleShader* _shader, 
	Texture* _bbTex, Texture* _decayTex,
	int avgLifetime, int varLifetime, 
	glm::vec4 initAcn, glm::vec4 initVel,
	int perturbChance, float perturbRadius,
	float baseRadius, float centerForce,
	float bbHeight, float bbWidth,
	bool perturb_on, bool _init_perturb)
	:AdvectParticlesLights(_maxParticles,
		_nLights, _shader, 
		_bbTex, _decayTex,
		avgLifetime, varLifetime, 
		initAcn, initVel,
		perturbChance, perturbRadius,
		baseRadius, centerForce,
		bbHeight, bbWidth,
		perturb_on, _init_perturb),
	 interval(_interval), counter(0),
	 clumpSize(_clumpSize)
{
	init();
}

void AdvectParticlesCentroidLights::init()
{
	for(int i = 0; i < nLights; ++i)
	{
		std::vector<int> clump;
		for(int j = 0; j < clumpSize; ++j)
			clump.push_back(randi(0, maxParticles));
		clumps.push_back(clump);
	}
}

void AdvectParticlesCentroidLights::randomizeClumps()
{
	for(auto i = clumps.begin(); i != clumps.end(); ++i)
		for(auto j = i->begin(); j != i->end(); ++j)
			(*j) = randi(0, maxParticles);
}

void AdvectParticlesCentroidLights::updateLights()
{
	if(interval == 0)
		randomizeClumps();
	else if(interval > 0)
	{
		counter += glutGet(GLUT_ELAPSED_TIME);
		if(counter > interval)
		{
			randomizeClumps();
			counter = 0;
		}
	}

	for(int i = 0; i < nLights; ++i)
	{
		lights[i]->setPos(modelToWorld * getParticleCentroid(clumps[i]));
	}
}

glm::vec4 AdvectParticlesCentroidLights::getParticleCentroid(
	const std::vector<int>& clump)
{
	glm::vec4 sum;
	for(auto i = clump.begin(); i != clump.end(); ++i)
		sum += particles[*i].pos;
	return sum / (float) clump.size();
}
