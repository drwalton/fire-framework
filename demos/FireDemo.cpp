#include "Scene.hpp"
#include "Camera.hpp"
#include "Texture.hpp"
#include "Particles.hpp"

#include <glm.hpp>
#include <GL/glut.h>
#include <gtc/matrix_transform.hpp>

#include <limits>

/* Fire Forces Demo
 * This demo applies a force to the fire which varies with time. This is 
 * intended to demonstrate the way the lighting dynamically adapts to the
 * behaviour of the flame.
 */

int init();
void display();
void reshape (int, int);
void keyboard(unsigned char, int, int);
int getNoParticles();

ParticleShader* sShader; // Static texture shader.
ParticleShader* tShader; // Scrolling texture shader.
ParticleShader* pShader; // Procedural texture shader.

AdvectParticles* adjust;
float initVel = 0.0001f;
float delInitVel = 0.00001f;
float centerForce = 0.000003f;
float delCenterForce = 0.0000001f;

Scene* scene;

const int k = 5;

int eTime;
int deTime;

float flameIntensity = 0.001f;
float intensityDelta = 0.0001f;

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
	/* Set ambient colour here. */
	const glm::vec4 ambColor(0.7f, 0.7f, 0.9f, 1.0f); //light blue

	glClearColor(ambColor.x, ambColor.y, ambColor.z, ambColor.w);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	scene = new Scene();

	pShader = new ParticleShader(true, false, "ProceduralFire");
	tShader = new ParticleShader(true, true , "ScrollTexFire" );
	sShader = new ParticleShader(true, true , "StaticTexFire");

	Texture* flameAlphaTex = new Texture("flameAlpha.png");
	Texture* staticFlameAlphaTex = new Texture("staticFlameAlpha.png");
	Texture* flameDecayTex = new Texture("flameDecay.png");

	std::cout << "Please choose demo:" << std::endl;
	std::cout << ">  1. Comparison of particle billboard rendering methods" << std::endl;
	std::cout << ">  2. Single flame with static textures." << std::endl;
	std::cout << ">  3. Single flame with scrolling textures." << std::endl;
	std::cout << ">  4. Single flame with procedural textures." << std::endl;

	int choice = -1;
	while(!(choice >= 1 && choice <= 4))
	{
		std::cout << std::endl;
		std::cout << "Please enter your choice: ";
		std::cin >> choice;
		if(!std::cin) std::cout << "Sorry, I didn't understand that."
			<< "Please enter an integer from 1 to 4, and press enter.\n";
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	
	if(choice == 1) // Billboard rendering method comparison
	{
		int nParticles = getNoParticles();
		const float height = -1.0f;
		const float spacing = 1.0f;

		AdvectParticles* flameStatic = 
			new AdvectParticles(nParticles, sShader, staticFlameAlphaTex, flameDecayTex, false);
		flameStatic->translate(glm::vec3(-spacing, height, 0.0f));
		scene->add(flameStatic);

		AdvectParticles* flameScrolling = 
			new AdvectParticles(nParticles, tShader, flameAlphaTex, flameDecayTex);
		flameScrolling->translate(glm::vec3(0.0f, height, 0.0f));
		scene->add(flameScrolling);

		AdvectParticles* flameProcedural = 
			new AdvectParticles(nParticles, pShader, flameAlphaTex, flameDecayTex);
		flameProcedural->translate(glm::vec3(spacing, height, 0.0f));
		scene->add(flameProcedural);
	}

	if(choice == 2 || choice == 3 || choice == 4)
	{
		const float height = -1.0f;

		int nParticles = getNoParticles();

		AdvectParticles* flame;

		if(choice == 2)
			flame = new AdvectParticles(
				nParticles, sShader, staticFlameAlphaTex, flameDecayTex, false);
		else if(choice == 3)
			flame = new AdvectParticles(
				nParticles, tShader, flameAlphaTex, flameDecayTex);
		else if(choice == 4)
			flame = new AdvectParticles(
				nParticles, pShader, flameAlphaTex, flameDecayTex);

		flame->translate(glm::vec3(0.0f, height, 0.0f));

		adjust = flame;

		scene->add(flame);
	}

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
	case 't':
		initVel += delInitVel;
		std::cout << initVel << std::endl;
		adjust->initVel = initVel;
		break;
	case 'g':
		initVel -= delInitVel;
		std::cout << initVel << std::endl;
		adjust->initVel = initVel;
		break;
	case 'y':
		centerForce += delCenterForce;
		std::cout << centerForce << std::endl;
		adjust->centerForce = centerForce;
		break;
	case 'h':
		centerForce -= delCenterForce;
		std::cout << centerForce << std::endl;
		adjust->centerForce = centerForce;
		break;
    }
}

int getNoParticles()
{
	int choice = -1;
	while(!(choice >= 1))
	{
		std::cout << std::endl;
		std::cout << "Please enter desired no. of particles (A few hundred is recommended): ";
		std::cin >> choice;
		if(!std::cin) std::cout << "Sorry, I didn't understand that."
			<< "Please enter a positive integer and press enter.\n";
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	return choice;
}
