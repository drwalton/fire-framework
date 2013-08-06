#include "Scene.hpp"
#include "Texture.hpp"
#include "Particles.hpp"
#include "Mesh.hpp"
#include "AOMesh.hpp"
#include "SH.hpp"
#include "SHMat.hpp"
#include "SphereFunc.hpp"

#include <glm.hpp>
#include <GL/glut.h>

/* This file will contain the construction and rendering of the scene
 * I am working on right now. 
 */

template <typename Fn>
void plotApproximations(Fn f, int nBands, float spacing, glm::vec3 translate)
{
	Shader* plotShader = new Shader(false, "SpherePlot");

	SpherePlot* original = new SpherePlot(		
	[&f] (double theta, double phi) -> float 
	{
		return f(theta, phi);
	}
	, 100, plotShader);

	scene->add(original);

	for(int i = 1; i < nBands; ++i)
	{
		std::vector<glm::vec3> proj = SH::shProject(20, i, 
		[&f] (double theta, double phi) -> glm::vec3 
		{
			return glm::vec3(f(theta, phi));
		}
		);

		SpherePlot* recovered = new SpherePlot(		
		[&proj] (double theta, double phi) -> float 
		{
			return SH::evaluate(proj, theta, phi).x;
		}
		, 50, plotShader);

		recovered->translate(glm::vec3(i * spacing, 0.0, 0.0));
		scene->add(recovered);
	}
}

int init();
void display();
void reshape (int, int);
void keyboard(unsigned char, int, int);

void addSHArray(Scene* scene, glm::vec3 pos, int nBands, float scale, float spacing);

const int nSwirls = 400;
const int nSparks = 5;

float theta = 0.0f;
float phi = 0.0f;

Scene* scene;
SHLight* light;
SpherePlot* plot;

const int k = 5;

int eTime;
int deTime;

const float delta = 0.4f;

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	eTime = glutGet(GLUT_ELAPSED_TIME);
    glutInitDisplayMode(GLUT_DOUBLE);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("API Demo");
    glewInit();
    int good = init();
    if(!good) return 0;
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
    glutMainLoop();

	delete scene;
}

// Called by glutInit().
int init()
{
	glClearColor(1.0, 1.0, 1.0, 1.0);

	/*
	std::vector<glm::vec4> c = SH::shProject(10, 3, 
		[] (double x, double y) -> glm::vec3 
		{
			return glm::vec3(1.0f);
		}
	);

	for(auto i = c.begin(); i != c.end(); ++i)
		std::cout << (*i).x << "\n";

	SHMat rot(glm::rotate(glm::mat4(1.0f), 2.0f, glm::vec3(1.0f, 0.0f, 0.0f)), 3);
	rot.print();

	c = rot * c;
	std::cout << "Rotated:\n";
	for(auto i = c.begin(); i != c.end(); ++i)
		std::cout << (*i).x << "\n";
	*/

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	scene = new Scene();
	/*
	ParticleShader* pShader = new ParticleShader(true, "ScrollTexFire");
	Texture* flameTex = new Texture("bigFlame.png");
	Texture* decayTex = new Texture("decay2.png");
	AdvectParticlesCentroidLights* centreParticles = 
		new AdvectParticlesCentroidLights(nSwirls, 10, 10, 1000, pShader, flameTex, decayTex);
	centreParticles->translate(glm::vec3(0.0, -1.0, 1.5));
	scene->add(centreParticles);

	AdvectParticlesRandLights* randParticles = new AdvectParticlesRandLights(nSwirls, 10, 2000, pShader, flameTex, decayTex);
	randParticles->translate(glm::vec3(0.0, -1.0, -3.0));
	
	scene->add(randParticles);
	*/

	/*
	AOShader* aoShader = new AOShader(false, "AOSolid");
	LightShader* lightShader = new LightShader(false, "Solid");

	auto loadedMesh = Mesh::loadFile("Rabbit.obj", COMBINED, lightShader);
	for(auto i = loadedMesh.begin(); i != loadedMesh.end(); ++i)
	{
		(*i)->uniformScale(2.0f);
		(*i)->translate(glm::vec3(0.0, -1.5, 5.0));
		scene->add(*i);
	}
	*/
	/*
	auto loadedAO = AOMesh::loadFile("Rabbit.obj", aoShader);

	for(auto i = loadedAO.begin();
		i != loadedAO.end(); ++i)
	{
		(*i)->uniformScale(2.0f);
		(*i)->translate(glm::vec3(0.0, -1.5, 0.0));
		scene->add(*i);
	}
	*/

	/*
	LightShader* lShader = new LightShader(false, "Solid");
	*/


	/*
	SHShader* shShader = new SHShader(false, "PRTfrag");
	
	std::vector<DiffPRTMesh*> loadedPRT = DiffPRTMesh::loadFile(
		UNSHADOWED, COMBINED, "bunny.obj", shShader);

	for(auto i = loadedPRT.begin();
		i != loadedPRT.end(); ++i)
	{
		(*i)->uniformScale(6.0f);
		(*i)->translate(glm::vec3(-2.0, -0.5, 0.0));
		scene->add(*i);
	}
	loadedPRT = DiffPRTMesh::loadFile(
		SHADOWED, COMBINED, "bunny.obj", shShader);

	for(auto i = loadedPRT.begin();
		i != loadedPRT.end(); ++i)
	{
		(*i)->uniformScale(6.0f);
		(*i)->translate(glm::vec3(0.0, -0.5, 0.0));
		scene->add(*i);
	}
	loadedPRT = DiffPRTMesh::loadFile(
		INTERREFLECTED, COMBINED, "bunny.obj", shShader);

	for(auto i = loadedPRT.begin();
		i != loadedPRT.end(); ++i)
	{
		(*i)->uniformScale(6.0f);
		(*i)->translate(glm::vec3(2.0, -0.5, 0.0));
		scene->add(*i);
	}
	*/

	/*
	loadedPRT = DiffPRTMesh::loadFile(
		SHADOWED, COMBINED, "Rabbit.obj", shShader);

	for(auto i = loadedPRT.begin();
		i != loadedPRT.end(); ++i)
	{
		(*i)->uniformScale(2.0f);
		(*i)->translate(glm::vec3(0.0, -1.5, 0.0));
		scene->add(*i);
	}

	loadedPRT = DiffPRTMesh::loadFile(
		INTERREFLECTED, COMBINED, "Rabbit.obj", shShader);

	for(auto i = loadedPRT.begin();
		i != loadedPRT.end(); ++i)
	{
		(*i)->uniformScale(2.0f);
		(*i)->translate(glm::vec3(2.0, -1.5, 0.0));
		scene->add(*i);
	}
	/*
	std::vector<Mesh*> loaded = Mesh::loadFile("Rabbit.obj", lShader);

	for(auto i = loaded.begin(); i != loaded.end(); ++i)
	{
		(*i)->uniformScale(2.0f);
		(*i)->translate(glm::vec3(-3.0, -1.5, 0.0));
		(*i)->setAmbient(glm::vec4(1.0, 0.0, 1.0, 1.0));
		(*i)->setDiffuse(glm::vec4(1.0, 0.0, 1.0, 1.0));
		scene->add(*i);
	}
	*/

	/*
	loaded = Mesh::loadFile("house plant.obj", lShader);

	for(auto i = loaded.begin(); i != loaded.end(); ++i)
	{
		(*i)->uniformScale(0.004f);
		(*i)->translate(glm::vec3(3.0, -1.0, 0.0));
		scene->add(*i);
	}
	*/

	/*
	Shader* plotShader = new Shader(false, "SpherePlot");

	plot = new SpherePlot(
		[] (double theta, double phi) -> float
		{
			//float val = 0.2f;
			float val = pulse(theta, phi, glm::vec3(1.0, 0.0, 0.0), 4.0f, 1.0f);

			return val;
		},
			20, plotShader);
	scene->add(plot);

	light = new SHLight(
		[] (double theta, double phi) -> glm::vec3 
		{
			//float val = 0.2f;
			float val = pulse(theta, phi, glm::vec3(1.0, 0.0, 0.0), 4.0f, 1.0f);

			return glm::vec3(val, val, val);
		}
	);
	scene->add(light);
	*/

	
	plotApproximations( 
	[] (float theta, float phi) -> float
	{
		return phi / (2 * PI);
	},
		8, 1.5f, glm::vec3(-7.0, 0.0, 0.0));
	
	
	//addSHArray(scene, glm::vec3(0.0f, -3.5, 0.0f), 7, 1.0f, 2.0f);

	return 1;
}

// Perform rendering and updates here.
void display()
{
	deTime = glutGet(GLUT_ELAPSED_TIME) - eTime;
	eTime = glutGet(GLUT_ELAPSED_TIME);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	scene->update(deTime);
	glm::mat4 rotation(1.0f);
	//Look up/down
	//rotation = glm::rotate(glm::mat4(1.0), phi, glm::vec3(1.0, 0.0, 0.0));
	//Spin around
	//rotation = glm::rotate(rotation,     theta, glm::vec3(0.0, 1.0, 0.0));
	//light->rotateCoeffts(rotation);
	//plot->setModelToWorld(rotation);
	//plot->translate(glm::vec3(0.0, 2.0, 0.0));
	scene->render();
	glutSwapBuffers();
	glutPostRedisplay();
}

// Called when window size changes.
void reshape (int w, int h)
{
	float aspect = (float) w / (float) h;
	scene->camera->setAspect(aspect);
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

// Called when a key is pressed.
void keyboard(unsigned char key, int x, int y)
{
	scene->camera->keyboardInput(key, x, y);

    switch (key)
    {
	case 't':
		theta += 1.6f;
		break;
	case 'g':
		theta -= 1.6f;
		break;
	case 'f':
		phi -= 1.6f;
		break;
	case 'h':
		phi += 1.6f;
		break;

    case 27:
        exit(0);
        return;
    }
}

// Renders an array of SH basis functions.
void addSHArray(Scene* scene, glm::vec3 pos, int nBands,
	float scale, float spacing)
{
	Shader* plotShader = new Shader(false, "SpherePlot");

	for(int l = 0; l < nBands; ++l)
		for(int m = -l; m <= l; ++m)
		{
			SpherePlot* plot = new SpherePlot(		
				[l, m] (double theta, double phi) -> float 
				{
					//float val = 0.2f;
					float val = SH::realSH(l, m, theta, phi);

					return val;
				}
				, 50, plotShader);
			plot->uniformScale(scale);
			plot->prependTransform(glm::rotate(
				glm::mat4(1.0f), 90.0f, glm::vec3(0.0, 1.0, 0.0)));
			plot->translate(pos + glm::vec3(m * spacing, l * spacing, 0.0f));
			
			scene->add(plot);
		}
}
