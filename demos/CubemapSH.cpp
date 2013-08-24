#include "Scene.hpp"
#include "Camera.hpp"
#include "Texture.hpp"
#include "Particles.hpp"
#include "Mesh.hpp"
#include "PRTMesh.hpp"

#include <glm.hpp>
#include <GL/glut.h>
#include <gtc/matrix_transform.hpp>

#include <fstream>

/* Cubemap SH Demo
 * A demo to illustrate projecting a particle-based fire as an SH lighting
 *   environment, via rendering to a cubemap.
 */

int init();
void display();
void reshape (int, int);
void keyboard(unsigned char, int, int);

std::string toString(int i);

ParticleShader* tShader; // Scrolling texture shader.
ParticleShader* pShader; // Procedural texture shader.
ParticleShader* sShader; // Spark shader.

AdvectParticlesSHCubemap* flame;
AdvectParticles*          sparks;
AdvectParticles*          smoke;

PRTMesh* bunny;

Scene* scene;
SHLight* light;

float flameIntensity = 1.8f;
float flameAmbIntensity = 0.15f;

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
    glutCreateWindow("Fire and SH Lighting Demo");
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

	/* Spark Properties */
	const int nSparkParticles = 5;
	const int sparkLifetime = 2000;
	const int sparkVarLifetime = 200;
	const glm::vec4 sparkInitAcn(0.0f, -0.0000004f, 0.0f, 0.0f);
	const glm::vec4 sparkInitVel(0.0f, 0.0008f, 0.0f, 0.0f);
	const int avgSparkPerturb = 1000;
	const int varSparkPerturb = 100;
	const float sparkPerturbRadius = 0.0004f;
	const float sparkCenterForce = 0.0f;
	const float sparkBaseRadius = 0.2f;
	const float sparkBBHeight = 0.03f;
	const float sparkBBWidth = 0.03f;

	/* Smoke Properties */
	const int nSmokeParticles = 20;

	/* Bunny Properties */
	const float bunnySpecExp = 1.0f;
	const PRTMode mode = UNSHADOWED;

	SHShader* bunnyShader = new SHShader(false, "diffPRT");

	Texture* bunnyDiffTex = new Texture("bunnyDiff.png");

	if(mode == UNSHADOWED)
	{	
		if(!fileExists("stanford.obj.prtu" + toString(GC::nSHBands)))
			PRTMesh::bake(UNSHADOWED, 
				"stanford.obj", "bunnyDiff.png", 
				GC::sqrtSHSamples, GC::nSHBands);

		bunny = new PRTMesh(
			"stanford.obj.prtu" + toString(GC::nSHBands), bunnyShader);
	}

	if(mode == SHADOWED)
	{	
		if(!fileExists("stanford.obj.prts" + toString(GC::nSHBands)))
			PRTMesh::bake(SHADOWED, 
				"stanford.obj", "bunnyDiff.png", 
				GC::sqrtSHSamples, GC::nSHBands);

		bunny = new PRTMesh(
			"stanford.obj.prts" + toString(GC::nSHBands), bunnyShader);
	}

	if(mode == INTERREFLECTED)
	{	
		if(!fileExists("stanford.obj.prti" + toString(GC::nSHBands)))
			PRTMesh::bake(INTERREFLECTED, 
				"stanford.obj", "bunnyDiff.png", 
				GC::sqrtSHSamples, GC::nSHBands);

		bunny = new PRTMesh(
			"stanford.obj.prti" + toString(GC::nSHBands), bunnyShader);
	}

	bunny->uniformScale(0.2f);

	scene->add(bunny);

	pShader = new ParticleShader(true, false, "ProceduralFire");
	tShader = new ParticleShader(true, true , "ScrollTexFire" );
	sShader = new ParticleShader(true, true , "Sparks");

	Texture* flameAlphaTex = new Texture("flameAlpha.png");
	Texture* flameDecayTex = new Texture("flameDecay.png");

	Texture* sparkAlphaTex = new Texture("sparkAlpha.png");
	Texture* sparkDecayTex = new Texture("sparkDecay.png");

	Texture* smokeAlphaTex = new Texture("smokeAlpha.png");
	Texture* smokeDecayTex = new Texture("smokeDecay.png");

	flame = new AdvectParticlesSHCubemap(
		bunny, nFlameParticles, pShader, flameIntensity, flameAlphaTex, flameDecayTex);

	sparks = new AdvectParticles(
		nSparkParticles, sShader, sparkAlphaTex, sparkDecayTex,
		sparkLifetime, sparkVarLifetime, 
		sparkInitAcn, sparkInitVel,
		avgSparkPerturb, varSparkPerturb, sparkPerturbRadius,
		sparkBaseRadius, sparkCenterForce,
		sparkBBHeight, sparkBBWidth,
		true, true, false);

	smoke = new AdvectParticles(
		nSmokeParticles, pShader, smokeAlphaTex, smokeDecayTex);

	flame->translate(glm::vec3(0.0f, 0.0f, 0.5f));
	sparks->translate(glm::vec3(0.0f, 0.0f, 0.5f));
	smoke->translate(glm::vec3(0.0f, 1.0f, 0.5f));

	scene->add(flame);
	scene->add(sparks);
	scene->add(smoke);

	flame->setIntensity(flameIntensity);
	flame->setAmbIntensity(flameAmbIntensity);

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
	case 'r':
		flame->saveCubemap();
		break;
	case 't':
		flameIntensity += 0.01f;
		flame->setIntensity(flameIntensity);
		std::cout << flameIntensity << std::endl;
		break;
	case 'g':
		flameIntensity -= 0.01f;
		flame->setIntensity(flameIntensity);
		std::cout << flameIntensity << std::endl;
		break;
	case 'y':
		flameAmbIntensity += 0.01f;
		flame->setAmbIntensity(flameAmbIntensity);
		std::cout << flameAmbIntensity << std::endl;
		break;
	case 'h':
		flameAmbIntensity -= 0.01f;
		flame->setAmbIntensity(flameAmbIntensity);
		std::cout << flameAmbIntensity << std::endl;
		break;
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

std::string toString(int i)
{
	return std::to_string(static_cast<long long>(i));
}
