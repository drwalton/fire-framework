#include "Scene.hpp"
#include "Texture.hpp"
#include "Particles.hpp"
#include "Mesh.hpp"
#include "SH.hpp"
#include "SHMat.hpp"

#include <glm.hpp>
#include <GL/glut.h>

/* This file will contain the construction and rendering of the scene
 * I am working on right now. 
 */

int init();
void display();
void reshape (int, int);
void keyboard(unsigned char, int, int);

const int nSwirls = 400;
const int nSparks = 5;

float angle = 0.0f;

Scene* scene;
SHLight* light;

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
	glClearColor(0.0, 0.0, 0.0, 1.0);
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


	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	scene = new Scene();
	
	ParticleShader* pShader = new ParticleShader(true, "ScrollTexFire");
	Texture* flameTex = new Texture("bigFlame.png");
	Texture* decayTex = new Texture("decay2.png");
	AdvectParticlesCentroidLights* centreParticles = 
		new AdvectParticlesCentroidLights(nSwirls, 10, 10, 1000, pShader, flameTex, decayTex);
	centreParticles->translate(glm::vec3(0.0, -1.0, 1.5));
	scene->add(centreParticles);

	AdvectParticlesRandLights* randParticles = new AdvectParticlesRandLights(nSwirls, 10, 2000, pShader, flameTex, decayTex);
	randParticles->translate(glm::vec3(0.0, -1.0, -3.0));
	//scene->add(randParticles);

	AOShader* aoShader = new AOShader(false, "AOSolid");
	LightShader* lightShader = new LightShader(false, "Solid");

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

	SHShader* shShader = new SHShader(false, "PRTfrag");
	
	/*
	std::vector<DiffPRTMesh*> loadedPRT = DiffPRTMesh::loadFile(true, "Rabbit.obj", shShader);

	for(auto i = loadedPRT.begin();
		i != loadedPRT.end(); ++i)
	{
		(*i)->uniformScale(2.0f);
		(*i)->translate(glm::vec3(0.0, -1.5, 0.0));
		//scene->add(*i);
	}
	*/
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
	light = new SHLight(
		[] (double theta, double phi) -> glm::vec3 
		{
			//float val = 0.2f;
			float val = theta > 2.0 ? 0.6f : 0.0f;

			return glm::vec3(val, val, val);
		}
	);
	scene->add(light);

	return 1;
}

// Perform rendering and updates here.
void display()
{
	deTime = glutGet(GLUT_ELAPSED_TIME) - eTime;
	eTime = glutGet(GLUT_ELAPSED_TIME);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	scene->update(deTime);
	light->rotateCoeffts(glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 1.0f, 0.0f)));
	angle += 1.0f;
	angle = angle > 360.0f ? angle - 360.0f : angle;
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
      	case 27:
            exit(0);
            return;
    }
}
