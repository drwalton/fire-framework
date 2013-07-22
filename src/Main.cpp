#include "Scene.hpp"
#include "SH.hpp"
#include "SHMat.hpp"

#include <glm.hpp>

/* This file will contain the construction and rendering of the scene
 * I am working on right now. 
 */

int init();
void display();
void reshape (int, int);
void keyboard(unsigned char, int, int);

const int nSwirls = 400;
const int nSparks = 5;

Scene* scene;

LightShader* lShader;
ParticleShader* pShader;
AdvectParticlesRandLights<nSwirls>* swirl;
AdvectParticles<nSwirls>* swirl2;
AdvectParticles<nSwirls>* swirl3;
AdvectParticles<nSparks>* sparks;
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

	delete lShader;
	delete pShader;
	delete scene;
}

// Called by glutInit().
int init()
{
	/*
	std::vector<float> c = SH::shProject(10, 3, [] (double x, double y) -> double {return SH::realSH(1,1,x,y) + SH::realSH(1,-1,x,y) + SH::realSH(2,1,x,y) + SH::realSH(2,-1,x,y)+ SH::realSH(2,2,x,y) + SH::realSH(2,-2,x,y) ;});
	for(std::vector<float>::iterator i = c.begin(); i != c.end(); ++i)
		std::cout << (*i) << "\n";
	SHMat rot(glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f)), 3);
	c = rot * c;
	std::cout << "Rotated:\n";
	for(std::vector<float>::iterator i = c.begin(); i != c.end(); ++i)
		std::cout << (*i) << "\n";
	*/
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND); 
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	scene = new Scene();

	pShader = new ParticleShader(true, "ScrollTexFire");
	Texture* flameTex = new Texture("bigFlame.png");
	Texture* decayTex = new Texture("decay2.png");
	AdvectParticlesCentroidLights<nSwirls>* centreParticles = new AdvectParticlesCentroidLights<nSwirls>(10, 10, 1000, pShader, flameTex, decayTex);
	centreParticles->translate(glm::vec3(0.0, -1.0, 3.0));
	scene->add(centreParticles);

	swirl = new AdvectParticlesRandLights<nSwirls>(10, 2000, pShader, flameTex, decayTex);
	swirl->translate(glm::vec3(0.0, -1.0, -3.0));
	scene->add(swirl);

	lShader = new LightShader(false, "Solid");
	SHShader* shShader = new SHShader(false, scene->maxSHLights, scene->nSHCoeffts, "PRTfrag");

	std::vector<DiffPRTMesh*> loadedPRT = DiffPRTMesh::loadFile("teapot.obj", 2, shShader);

	for(std::vector<DiffPRTMesh*>::iterator i = loadedPRT.begin();
		i != loadedPRT.end(); ++i)
	{
		(*i)->uniformScale(0.08f);
		scene->add(*i);
	}

	std::vector<Mesh*> loaded = Mesh::loadFile("Rabbit.obj", lShader);

	for(std::vector<Mesh*>::iterator i = loaded.begin();
		i != loaded.end(); ++i)
	{
		(*i)->uniformScale(2.0f);
		(*i)->translate(glm::vec3(-3.0, -1.5, 0.0));
		(*i)->setAmbient(glm::vec4(1.0, 0.0, 1.0, 1.0));
		(*i)->setDiffuse(glm::vec4(1.0, 0.0, 1.0, 1.0));
		scene->add(*i);
	}

	loaded = Mesh::loadFile("house plant.obj", lShader);

	for(std::vector<Mesh*>::iterator i = loaded.begin();
		i != loaded.end(); ++i)
	{
		(*i)->uniformScale(0.004f);
		(*i)->translate(glm::vec3(3.0, -1.0, 0.0));
		scene->add(*i);
	}

	SHLight* light = new SHLight(
		[] (double theta, double phi) -> glm::vec3 
		{
			float val = (theta + phi) < 1.0 ? 3.0 : 0.0;
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
