#include "Scene.hpp"
#include "Camera.hpp"
#include "Texture.hpp"
#include "Particles.hpp"
#include "Mesh.hpp"

#include <glm.hpp>
#include <GL/glut.h>
#include <gtc/matrix_transform.hpp>

/* This file will contain the construction and rendering of the scene
 * I am working on right now. 
 */

int init();
void display();
void reshape (int, int);
void keyboard(unsigned char, int, int);

ParticleShader* tShader; // Scrolling texture shader.
ParticleShader* pShader; // Procedural texture shader.
ParticleShader* sShader; // Spark shader.

AdvectParticlesCentroidLights* flame;
AdvectParticles*               sparks;
AdvectParticles*               smoke;

Mesh* bunny;

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
    glutCreateWindow("Fire and Phong Lighting Demo");
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
	glClearColor(0.7f, 0.7f, 0.9f, 1.0f); // Light blue

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	scene = new Scene();

	/* Flame Properties */
	const int nFlameParticles = 400;
	const int nFlameLights = 10;
	const int lightClumpSize = 10;
	const int hopInterval = -1; // Never hop. Set to +ve ms value to hop.

	/* Spark Properties */
	const int nSparkParticles = 5;
	const int sparkLifetime = 2000;
	const int sparkVarLifetime = 200;
	const glm::vec4 sparkInitAcn(0.0f, -0.0000004f, 0.0f, 0.0f);
	const glm::vec4 sparkInitVel(0.0f, 0.0008f, 0.0f, 0.0f);
	const int sparkPerturbChance = 10;
	const float sparkPerturbRadius = 0.0004f;
	const float sparkCenterForce = 0.0f;
	const float sparkBaseRadius = 0.2f;
	const float sparkBBHeight = 0.03f;
	const float sparkBBWidth = 0.03f;

	/* Smoke Properties */
	const int nSmokeParticles = 20;

	/* Bunny Properties */
	const float bunnySpecExp = 1.0f;

	pShader = new ParticleShader(true, false, "ProceduralFire");
	tShader = new ParticleShader(true, true , "ScrollTexFire" );
	sShader = new ParticleShader(true, true , "Sparks");

	Texture* flameAlphaTex = new Texture("flameAlpha.png");
	Texture* flameDecayTex = new Texture("flameDecay.png");

	Texture* sparkAlphaTex = new Texture("sparkAlpha.png");
	Texture* sparkDecayTex = new Texture("sparkDecay.png");

	Texture* smokeAlphaTex = new Texture("smokeAlpha.png");
	Texture* smokeDecayTex = new Texture("smokeDecay.png");

	flame = new AdvectParticlesCentroidLights(
		nFlameParticles, nFlameLights, lightClumpSize, hopInterval,
		pShader, flameAlphaTex, flameDecayTex);

	sparks = new AdvectParticles(
		nSparkParticles, sShader, sparkAlphaTex, sparkDecayTex,
		sparkLifetime, sparkVarLifetime, 
		sparkInitAcn, sparkInitVel,
		sparkPerturbChance, sparkPerturbRadius,
		sparkBaseRadius, sparkCenterForce,
		sparkBBHeight, sparkBBWidth,
		true, true);

	smoke = new AdvectParticles(
		nSmokeParticles, pShader, smokeAlphaTex, smokeDecayTex);

	flame->translate(glm::vec3(0.0f, 0.0f, 1.0f));
	sparks->translate(glm::vec3(0.0f, 0.0f, 1.0f));
	smoke->translate(glm::vec3(0.0f, 1.0f, 1.0f));

	scene->add(flame);
	scene->add(sparks);
	scene->add(smoke);

	LightShader* bunnyShader = new LightShader(false, "BlinnPhong");

	Texture* bunnyAmbTex = new Texture("bunnyAmb.png");
	Texture* bunnyDiffTex = new Texture("bunnyDiff.png");
	Texture* bunnySpecTex = new Texture("bunnySpec.png");

	bunny = new Mesh(
		"bunny.obj",
		bunnyAmbTex, bunnyDiffTex, bunnySpecTex,
		bunnySpecExp, bunnyShader);

	bunny->uniformScale(0.2f);

	scene->add(bunny);

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
    case 'f':
    	//Switch fire mode.
    	if(flame->getShader() == tShader)
    	{
    		flame->setShader(pShader);
    		smoke->setShader(pShader);
    	}
    	else
    	{
    		flame->setShader(tShader);
    		smoke->setShader(tShader);
    	}
    	break;

    case 27:
        exit(0);
        return;
    }
}
