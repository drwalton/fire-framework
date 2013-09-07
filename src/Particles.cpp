#include "Particles.hpp"

#include "Texture.hpp"
#include "Scene.hpp"
#include "SphereFunc.hpp"
#include "Shader.hpp"

#include <SOIL.h>
#include <GL/glut.h>

#include <gtc/matrix_transform.hpp>

#include<algorithm>

const float AdvectParticlesLights::minColor = 0.6f;
const float AdvectParticlesSHLights::minColor = 0.7f;
const glm::mat4 AdvectParticlesSHCubemap::turnAround = 
	glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));

AdvectParticles::AdvectParticles(int maxParticles,
	ParticleShader* shader, 
	Texture* bbTex, Texture* decayTex, bool texScrolls, bool additive)
	:ParticleSystem(maxParticles, shader),
	 bbTex(bbTex), decayTex(decayTex),
	 avgLifetime(3000), varLifetime(200),
	 avgPerturbTime(1000), varPerturbTime(100),
	 initAcn(glm::vec4(0.0, 0.0000004, 0.0, 0.0)),
	 initVel(0.001f),
	 initUpVel(0.0f),
	 perturbRadius(0.0001f),
	 centerForce(6e-7f),
	 baseRadius(0.2f),
	 bbHeight(0.3f), bbWidth(0.3f),
	 extForce(glm::vec4(0.0f)),
	 perturbOn(true), initPerturb(false),
	 cameraDir(glm::vec3(0.0, 0.0, -1.0)),
	 additive(additive), height(initAcn.y * avgLifetime)
{init(bbTex, decayTex, texScrolls);}

void AdvectParticles::init(Texture* bbTex, Texture* decayTex, bool texScrolls)
{
	shader->use();

	// Set up particles.
	for(int i = 0; i < maxParticles; ++i)
	{
		AdvectParticle p;
		p.pos = randInitPos();
		p.decay = 0.0f;
		p.randTex = randf(0.0f, 1.0f);
		particles.push_back(p);

		time.push_back(0);
		// Evenly spacing lifetimes so system stabilises quicly.
		lifeTime.push_back((avgLifetime * i) / maxParticles);
		acn.push_back(initAcn);
		
		perturbCounter.push_back(0);
		perturbTime.push_back(avgPerturbTime + randi(-varPerturbTime, varPerturbTime)); 

		if(initPerturb) vel.push_back(perturb(getInitVel(p.pos)));
		else vel.push_back(getInitVel(p.pos));
	}

	glUseProgram(0);

	shader->setAlpha(alpha);
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
	if(texScrolls) randTex_attrib = shader->getAttribLoc("vRandTex");

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, particles_vbo);
	glEnableVertexAttribArray(pos_attrib);
	glEnableVertexAttribArray(decay_attrib);
	glEnableVertexAttribArray(randTex_attrib);
	glVertexAttribPointer(pos_attrib, 4, GL_FLOAT, GL_FALSE, sizeof(AdvectParticle),
		reinterpret_cast<GLvoid*>(offsetof(AdvectParticle, pos)));
	glVertexAttribPointer(decay_attrib, 1, GL_FLOAT, GL_FALSE, sizeof(AdvectParticle), 
		reinterpret_cast<GLvoid*>(offsetof(AdvectParticle, decay)));
	glVertexAttribPointer(randTex_attrib, 1, GL_FLOAT, GL_FALSE, sizeof(AdvectParticle), 
		reinterpret_cast<GLvoid*>(offsetof(AdvectParticle, randTex)));

	glBindVertexArray(0);
}

void AdvectParticles::render()
{
	if(!scene) return;
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND); 
	if(additive)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	else
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shader->setModelToWorld(modelToWorld);
	shader->setAlpha(alpha);
	shader->setBBTexUnit(bbTex->getTexUnit());
	shader->setDecayTexUnit(decayTex->getTexUnit());
	shader->setBBHeight(bbHeight);
	shader->setBBWidth(bbWidth);

	shader->use();

	glBindBuffer(GL_ARRAY_BUFFER, particles_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particles.size()*sizeof(AdvectParticle),
		particles.data());

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glDrawArrays(GL_POINTS, 0, particles.size());

	glBindVertexArray(0);

	glUseProgram(0);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

void AdvectParticles::setShader(ParticleShader* shader)
{
	this->shader = shader;

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

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, particles_vbo);
	glEnableVertexAttribArray(pos_attrib);
	glEnableVertexAttribArray(decay_attrib);
	glEnableVertexAttribArray(randTex_attrib);
	glVertexAttribPointer(pos_attrib, 4, GL_FLOAT, GL_FALSE, sizeof(AdvectParticle),
		reinterpret_cast<GLvoid*>(offsetof(AdvectParticle, pos)));
	glVertexAttribPointer(decay_attrib, 1, GL_FLOAT, GL_FALSE, sizeof(AdvectParticle), 
		reinterpret_cast<GLvoid*>(offsetof(AdvectParticle, decay)));
	glVertexAttribPointer(randTex_attrib, 1, GL_FLOAT, GL_FALSE, sizeof(AdvectParticle), 
		reinterpret_cast<GLvoid*>(offsetof(AdvectParticle, randTex)));

	glBindVertexArray(0);
}

void AdvectParticles::update(int dTime)
{
	if(dTime > 1000) return; // Avoid updating if timestep excessive
	#pragma omp parallel for
	for(int i = 0; i < maxParticles; ++i)
		updateParticle(i, dTime);
}

void AdvectParticles::updateParticle(int index, int dTime)
{
	time[index] += dTime;
	if(time[index] > lifeTime[index]) spawnParticle(index);
	particles[index].decay = ((float) time[index]) / ((float) lifeTime[index]);
	perturbCounter[index] += dTime;

	if(perturbCounter[index] >= perturbTime[index] && perturbOn)
	{
		perturbCounter[index] = 0;
		perturbTime[index] = avgPerturbTime + randi(-varPerturbTime, varPerturbTime);
		vel[index] = perturb(vel[index]);
	}

	vel[index] += static_cast<float>(dTime) *
		(acn[index] + (glm::vec4(
			-particles[index].pos.x,
			0.0,
			-particles[index].pos.z,
			0.0)
			* centerForce));

	vel[index] += static_cast<float>(dTime) * extForce;
	particles[index].pos += (float) dTime * vel[index];
}

void AdvectParticles::spawnParticle(int index)
{
	time[index] = 0;
	lifeTime[index] = avgLifetime + randi(-varLifetime, +varLifetime);
	perturbCounter[index] = 0;
	perturbTime[index] = avgPerturbTime + randi(-varPerturbTime, varPerturbTime);
	particles[index].decay = 0.0;
	acn[index] = initAcn;
	particles[index].pos = randInitPos();
	vel[index] = getInitVel(particles[index].pos);
	particles[index].randTex = randf(0.0f, 1.0f);
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

glm::vec4 AdvectParticles::getInitVel(const glm::vec4& pos)
{
	return glm::vec4(pos.x, pos.y, pos.z, 0.0f) * initVel +
		glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) * initUpVel;

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

std::vector<glm::vec4> AdvectParticles::loadImage(const std::string& filename)
{
	std::string fullPath = "../textures/" + filename;

	int width, height, channels;
	unsigned char* data = SOIL_load_image(
		fullPath.c_str(),
		&width, &height, &channels,
		SOIL_LOAD_RGB);

	std::vector<glm::vec4> image;

	for(int p = 0; p < width; ++p)
	{
		glm::vec4 pixel;
		pixel.x = static_cast<float>(data[p*channels + 0]) / 255.0f;
		pixel.y = static_cast<float>(data[p*channels + 1]) / 255.0f;
		pixel.z = static_cast<float>(data[p*channels + 2]) / 255.0f;
		pixel.w = 1.0f;
		pixel = glm::clamp(pixel, glm::vec4(0.0f), glm::vec4(1.0f));
		image.push_back(pixel);
	}

	SOIL_free_image_data(data);

	return image;
}

float AdvectParticles::saturate(float val, float min)
{
	return glm::mix(min, 1.0f, val);
}

const float AdvectParticlesLights::specIntensity = 0.0001f;

AdvectParticlesLights::AdvectParticlesLights(int _maxParticles, int _nLights, 
	ParticleShader* _shader, Texture* _bbTex, Texture* _decayTex)
	:AdvectParticles(_maxParticles, _shader, _bbTex, _decayTex),
	 nLights(_nLights), lightIntensity(0.0014f)
{
	// Set up vector of lights.
	for(int i = 0; i < nLights; ++i)
	{
		PhongLight* light = new PhongLight(getOrigin());
		light->setSpecular(glm::vec4(specIntensity, specIntensity, specIntensity, 1.0f));
		lights.push_back(light);
	}
	particleColors = loadImage(decayTex->filename);
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

void AdvectParticlesLights::setLightIntensity(float lightIntensity)
{
	this->lightIntensity = lightIntensity / lights.size();
}

glm::vec4 AdvectParticlesLights::getParticleColor(float decay)
{
	int pixel = static_cast<int>(decay * (particleColors.size()-1));

	float decayIntensity = decay < 0.3 ? decay : (1 - decay);
	decayIntensity *= lightIntensity;

	return glm::vec4(
			saturate(particleColors[pixel].x, minColor) * decayIntensity,
			saturate(particleColors[pixel].y, minColor) * decayIntensity,
			saturate(particleColors[pixel].z, minColor) * decayIntensity,
			1.0f);
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
		lights[i]->setColor(getAverageColor(clumps[i]));
	}
}

glm::vec4 AdvectParticlesCentroidLights::getParticleCentroid(
	const std::vector<int>& clump)
{
	glm::vec4 sum;
	for(auto i = clump.begin(); i != clump.end(); ++i)
		sum += particles[*i].pos;
	return sum / static_cast<float>(clump.size());
}

glm::vec4 AdvectParticlesCentroidLights::getAverageColor(
	const std::vector<int>& clump)
{
	glm::vec4 color;
	for(auto i = clump.begin(); i != clump.end(); ++i)
		color += getParticleColor(particles[*i].decay);
	return color / static_cast<float>(clump.size());
}

AdvectParticlesSHLights::AdvectParticlesSHLights(
	Renderable* targetObj,
	float intensity,
	int maxParticles, int nLights, 
	ParticleShader* shader, 
	Texture* bbTex, Texture* decayTex)
	:AdvectParticles(maxParticles, shader, bbTex, decayTex),
	 nLights(nLights), targetObj(targetObj),
	 intensity(intensity / nLights)
{ makeLights(); }

void AdvectParticlesSHLights::makeLights()
{
	// Set up vector of lights.
	for(int i = 0; i < nLights; ++i)
	{
		lights.push_back(new SHLight(
			[] (float theta, float phi) -> glm::vec3 
			{
			//float val = 0.2f;
			float val = pulse(theta, phi, glm::vec3(1.0f, 0.0f, 0.0f), 5.0f, 1.0f);

			return glm::vec3(val, val, val);
			}
		));
	}

	particleColors = loadImage(decayTex->filename);
}

void AdvectParticlesSHLights::onAdd()
{
	SHLight* p;
	// Add lights
	for(auto l = lights.begin(); l != lights.end(); ++l)
	{
		p = scene->add(*l);
		if(p == nullptr) //Light not added correctly (too many lights?).
		{
			std::cout << "Warning: Not all SH lights could be added.\n";
			break;
		}
		(*l)->setIntensity(intensity);
	}
}

void AdvectParticlesSHLights::onRemove()
{
	for(auto l = lights.begin(); l != lights.end(); ++l)
		scene->remove(*l);
}

void AdvectParticlesSHLights::update(int dTime)
{
	AdvectParticles::update(dTime);
	updateLights();
}

void AdvectParticlesSHLights::setIntensity(float intensity)
{
	this->intensity = intensity;
	for(auto l = lights.begin(); l != lights.end(); ++l)
		(*l)->setIntensity(intensity);
}

glm::vec3 AdvectParticlesSHLights::getParticleColor(float decay)
{
	int pixel = static_cast<int>(decay * (particleColors.size()-1));
	return glm::vec3(
			saturate(particleColors[pixel].x, minColor),
			saturate(particleColors[pixel].y, minColor),
			saturate(particleColors[pixel].z, minColor));
}

AdvectParticlesCentroidSHLights::AdvectParticlesCentroidSHLights(
	Renderable* targetObj, float intensity,
	int _maxParticles,
	int _nLights, int _clumpSize,
	int _interval, ParticleShader* _shader, 
	Texture* _bbTex, Texture* _decayTex)
	:AdvectParticlesSHLights(
		targetObj, intensity,
		_maxParticles, 
		_nLights, _shader, 
		_bbTex, _decayTex),
	 interval(_interval), counter(0),
	 clumpSize(_clumpSize)
{ init(); }

void AdvectParticlesCentroidSHLights::init()
{
	for(int i = 0; i < nLights; ++i)
	{
		std::vector<int> clump;
		for(int j = 0; j < clumpSize; ++j)
			clump.push_back(randi(0, maxParticles));
		clumps.push_back(clump);
	}
}

void AdvectParticlesCentroidSHLights::randomizeClumps()
{
	for(auto i = clumps.begin(); i != clumps.end(); ++i)
		for(auto j = i->begin(); j != i->end(); ++j)
			(*j) = randi(0, maxParticles);
}

void AdvectParticlesCentroidSHLights::updateLights()
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

	glm::mat4 toTarget = glm::inverse(targetObj->getModelToWorld()) * modelToWorld;

	for(int i = 0; i < nLights; ++i)
	{
		lights[i]->pointAt(glm::vec3(toTarget * getParticleCentroid(clumps[i])));
		lights[i]->setColor(getAverageColor(clumps[i]));
	}
}

glm::vec4 AdvectParticlesCentroidSHLights::getParticleCentroid(
	const std::vector<int>& clump)
{
	glm::vec4 sum;
	for(auto i = clump.begin(); i != clump.end(); ++i)
		sum += particles[*i].pos;
	return sum / (float) clump.size();
}

glm::vec3 AdvectParticlesCentroidSHLights::getAverageColor(
	const std::vector<int>& clump)
{
	glm::vec3 color;
	for(auto i = clump.begin(); i != clump.end(); ++i)
		color += getParticleColor(particles[*i].decay);
	return color / static_cast<float>(clump.size());
}

AdvectParticlesSHCubemap::AdvectParticlesSHCubemap(
	Renderable* targetObj,
	int maxParticles, ParticleShader* shader, 
	float intensity,
	Texture* bbTex, Texture* decayTex)
	:AdvectParticles(maxParticles, shader, bbTex, decayTex),
	 targetObj(targetObj), intensity(intensity), 
	 clearColor(glm::vec4(0.0f)), ambColor(glm::vec4(0.0f))
{ init(); }

void AdvectParticlesSHCubemap::update(int dTime)
{
	AdvectParticles::update(dTime);
	renderCubemap();
	updateLight();
}

void AdvectParticlesSHCubemap::onAdd()
{
	renderCubemap();
	light = scene->add(new SHLight(
			[this] (float theta, float phi) -> glm::vec3
			{
				return this->cubemapLookup(theta, phi);
			}));

	amb = scene->add(new SHLight(
			[this] (float theta, float phi) -> glm::vec3
			{
				return glm::vec3(ambColor);
			}));

	if(light == nullptr || amb == nullptr)
		std::cout << "Warning: SH lights could not all be added.\n"; 
}

void AdvectParticlesSHCubemap::saveCubemap()
{
	saveFlag = true;
}

void AdvectParticlesSHCubemap::setIntensity(float intensity)
{
	this->intensity = intensity;
	light->setIntensity(intensity);
}

void AdvectParticlesSHCubemap::setAmbIntensity(float ambIntensity)
{
	this->ambIntensity = ambIntensity;
	amb->setIntensity(ambIntensity);
}

void AdvectParticlesSHCubemap::init()
{
	cubemapShader = new CubemapShader(true, false, "FireLight");

	glGenFramebuffers(1, &framebuffer);
	
	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, GC::cubemapSize, GC::cubemapSize);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	cube_pos_attrib = cubemapShader->getAttribLoc("vPos");
	cube_decay_attrib = cubemapShader->getAttribLoc("vDecay");

	glGenVertexArrays(1, &cube_vao);
	glBindVertexArray(cube_vao);

	glBindBuffer(GL_ARRAY_BUFFER, particles_vbo);
	glEnableVertexAttribArray(cube_pos_attrib);
	glEnableVertexAttribArray(cube_decay_attrib);
	glVertexAttribPointer(cube_pos_attrib, 4, GL_FLOAT, GL_FALSE, sizeof(AdvectParticle),
		reinterpret_cast<GLvoid*>(offsetof(AdvectParticle, pos)));
	glVertexAttribPointer(cube_decay_attrib, 1, GL_FLOAT, GL_FALSE, sizeof(AdvectParticle), 
		reinterpret_cast<GLvoid*>(offsetof(AdvectParticle, decay)));

	glBindVertexArray(0);

	saveFlag = false;
}

void AdvectParticlesSHCubemap::renderCubemap()
{
	if(!scene || !targetObj) return;

	// Store current state
	GLfloat clearCol[4];
	GLint viewport[4];
	GLboolean faceCull = GL_TRUE;
	GLboolean blend = GL_FALSE;
	GLint blendFn = GL_ONE;
	glGetFloatv(GL_COLOR_CLEAR_VALUE, clearCol); 
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetBooleanv(GL_CULL_FACE, &faceCull);
	glGetBooleanv(GL_BLEND, &blend);
	glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendFn);

	//Set new state
	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);

	glViewport(0, 0, GC::cubemapSize, GC::cubemapSize);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	//Set uniforms
	cubemapShader->setModelToWorld(modelToWorld);
	cubemapShader->setBBTexUnit(bbTex->getTexUnit());
	cubemapShader->setDecayTexUnit(decayTex->getTexUnit());
	cubemapShader->setBBWidth(bbWidth);
	cubemapShader->setBBHeight(bbHeight);
	glm::mat4 worldToObject = glm::inverse(
		targetObj->getTranslation() * targetObj->getRotation());
	cubemapShader->setWorldToObject(worldToObject);
	cubemapShader->setAlpha(1.0f);

	cubemapShader->use();

	glBindVertexArray(cube_vao);

	for(int face = 0; face < 6; ++face)
	{
		cubemapShader->setRotation(getRotation(face));

		glClear(GL_COLOR_BUFFER_BIT);

		glDrawArrays(GL_POINTS, 0, particles.size());

		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(0, 0, GC::cubemapSize, GC::cubemapSize, 
			GL_RGBA, GL_FLOAT, (cubemap[face]).data());

		if(saveFlag)
		{
			std::string filename = "cubemap" + std::to_string((long long) face) + ".bmp";
			
			unsigned char* img = (unsigned char*) malloc(GC::cubemapPixels * 3);
			unsigned char* imgFlip = (unsigned char*) malloc(GC::cubemapPixels * 3);

			glReadPixels(0, 0, GC::cubemapSize, GC::cubemapSize, 
				GL_RGB, GL_UNSIGNED_BYTE, img);

			for(int r = 0; r < GC::cubemapSize; ++r)
				for(int c = 0; c < GC::cubemapSize; ++c)
					for(int col = 0; col < 3; ++col)
					{
						imgFlip[(r*GC::cubemapSize + c)*3 + col] = 
							img[(((GC::cubemapSize-1) - r)*GC::cubemapSize + c)*3 + col];
					}

			SOIL_save_image(
					filename.c_str(),
					SOIL_SAVE_TYPE_BMP,
					GC::cubemapSize, GC::cubemapSize, 3,
					imgFlip
				);

			free(img);
			free(imgFlip);

			if(face == 5) saveFlag = false;
		}
	}

	glBindVertexArray(0);

	//Restore state
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(clearCol[0], clearCol[1], clearCol[2], clearCol[3]);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	if(faceCull == GL_TRUE) glEnable(GL_CULL_FACE);
	else glDisable(GL_CULL_FACE);
	if(blend == GL_TRUE) glEnable(GL_BLEND);
	else glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, blendFn);

	glUseProgram(0);
}

void AdvectParticlesSHCubemap::updateLight()
{
	// Set light coeffts to SH projection of cubemap.
	light->setCoeffts(
		SH::shProject(GC::sqrtSHSamples, GC::nSHBands,
			[this] (float theta, float phi) -> glm::vec3
			{
				return this->cubemapLookup(theta, phi);
			}
	));
}

glm::vec3 AdvectParticlesSHCubemap::cubemapLookup(float theta, float phi)
{
	glm::vec3 dir(
		sin(theta) * cos(phi),
		sin(theta) * sin(phi),
		cos(theta));

	int face = findFace(dir);
	float s, t;

	// Find texture coordinates.
	switch(face)
	{
	case 0: //+ve x
		s = -dir.z / dir.x;
		t = dir.y / dir.x;
		break;
	case 1: //-ve x
		s = dir.z / dir.x;
		t = dir.y / dir.x;
		break;
	case 2: //+ve y
		s = dir.x / dir.y;
		t = -dir.z / dir.y;
		break;
	case 3: //-ve y
		s = dir.x / dir.y;
		t = dir.z / dir.y;
		break;
	case 4: //+ve z
		s = dir.x / dir.z;
		t = dir.y / dir.z;
		break;
	case 5: //-ve z
		s = -dir.x / dir.z;
		t = dir.y / dir.z;
		break;
	}

	//Move from [-1,1] range to [0,1] range.
	s = (s + 1.0f) / 2.0f;
	t = (t + 1.0f) / 2.0f;

	//Find integer pixel coords:
	int s_p = static_cast<int>(s * (GC::cubemapSize-1));
	int t_p = static_cast<int>(t * (GC::cubemapSize-1));

	return glm::vec3(cubemap[face][s_p + t_p*GC::cubemapSize]);
}

int AdvectParticlesSHCubemap::findFace(glm::vec3 dir)
{
	if(abs(dir.x) >= abs(dir.y) && abs(dir.x) >= abs(dir.z)) //x
	{
		return dir.x >= 0.0f ? 0 : 1;
	}
	else if(abs(dir.y) >= abs(dir.z)) //y
	{
		return dir.y >= 0.0f ? 2 : 3;
	}
	else //z
	{
		return dir.z >= 0.0f ? 4 : 5;
	}
}

glm::mat4 AdvectParticlesSHCubemap::getRotation(int face)
{
	if(face == 0) //+ve x
		return glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	else if (face == 1) //-ve x
		return glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	else if (face == 2) //+ve y
		return glm::rotate(turnAround, 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	else if (face == 3) //-ve y
		return glm::rotate(turnAround, -90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	else if (face == 4) //+ve z
		return turnAround;
	else if (face == 5) //-ve z
		return glm::mat4(1.0f);
	else return glm::mat4(0.0f); //fallback
}
