#include "Scene.hpp"
#include "Camera.hpp"
#include "Texture.hpp"
#include "Particles.hpp"
#include "Mesh.hpp"
#include "PRTMesh.hpp"
#include "UserInput.hpp"

#include <glm.hpp>
#include <GL/glut.h>
#include <gtc/matrix_transform.hpp>

#include <fstream>

/* Fire-Lighting Demo
 */

int init();
void display();
void reshape (int, int);
void keyboard(unsigned char, int, int);

std::string toString(int i);

int choice = -1;

AdvectParticlesSHCubemap* cubemapFlame;
AdvectParticlesCentroidLights* phongFlame;
AdvectParticlesCentroidSHLights* shFlame;

PRTMesh* prtBunny;
Mesh* phongBunny;

Scene* scene;

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
	const int nFlameParticles = 500;
	const int groupSize = 10;
	const float flameIntensity = 2.0f;

	/* Bunny Properties */
	const float bunnySpecExp = 1.0f;
	const PRTMode mode = SHADOWED;

	std::cout << "Please enter fire-lighting combination technique:" << std::endl;
	std::cout << ">  1. Phong with individual particles" << std::endl;
	std::cout << ">  2. Phong with particle groups" << std::endl;
	std::cout << ">  3. SH with individual particles" << std::endl;
	std::cout << ">  4. SH with particle groups" << std::endl;
	std::cout << ">  5. SH wtih cubemap" << std::endl;

	choice = UserInput::getInt(1,5,"Please enter your choice:");

	int nLights;
	if(choice != 5) 
		nLights = UserInput::getInt(0, GC::maxPhongLights, 
			"Please enter desired no. of lights: ");

	Texture* flameAlphaTex = new Texture("flameAlpha.png");
	Texture* flameDecayTex = new Texture("flameDecay.png");
	ParticleShader* flameShader = new ParticleShader(true, true, "ScrollTexFire");

	glm::vec3 flamePos(0.5f, -0.7f, 0.0f);
	glm::vec3 bunnyPos(-0.5f, -0.5f, 0.0f);
	

	if(choice == 1 || choice == 2) // Blinn-Phong
	{
		LightShader* bunnyShader = new LightShader(false, "BlinnPhong");
		Texture* bunnyDiffTex = new Texture("bunnyDiff.png");
		Texture* bunnySpecTex = new Texture("bunnySpec.png");
		phongBunny = new Mesh("stanford.obj", 
			bunnyDiffTex, bunnyDiffTex, bunnySpecTex, 
			bunnySpecExp, bunnyShader);

		phongFlame = new AdvectParticlesCentroidLights(
			nFlameParticles, nLights, 
			choice == 1 ? 1 : groupSize, -1, 
			flameShader, flameAlphaTex, flameDecayTex);

		phongFlame->translate(flamePos);
		
		phongBunny->translate(bunnyPos);
		phongBunny->uniformScale(0.2f);

		scene->add(phongBunny); scene->add(phongFlame);
	}

	else if(choice == 3 || choice == 4 || choice == 5)
	{
		const std::string filename = "stanford.obj";
		const std::string diffTexture = "stanford.png";
		const PRTMode mode = SHADOWED;

		/* Check if baked file exists. If not, make one. */
		const std::string bakedFilename = filename + ".prt" + 
			(mode == UNSHADOWED ? "u" : mode == SHADOWED ? "s" : "i")
			+ "5";
		std::ifstream temp(bakedFilename);
		if(!temp)
			PRTMesh::bake(mode, filename, diffTexture, 40, 5, 5);

		SHShader* bunnyShader = new SHShader(false, "diffPRT");

		prtBunny = new PRTMesh(bakedFilename, bunnyShader);

		prtBunny->translate(bunnyPos);
		prtBunny->uniformScale(0.2f);

		scene->add(prtBunny);
	}

	if(choice == 3 || choice == 4)
	{
		shFlame = new AdvectParticlesCentroidSHLights(
			prtBunny, flameIntensity,
			nFlameParticles, nLights,
			choice == 4 ? 1 : groupSize, -1,
			flameShader, flameAlphaTex, flameDecayTex);

		shFlame->translate(flamePos);

		scene->add(shFlame);
	}

	else if(choice == 5)
	{
		cubemapFlame = new AdvectParticlesSHCubemap(
			prtBunny, nFlameParticles, flameShader,
			flameIntensity, flameAlphaTex, flameDecayTex);

		cubemapFlame->translate(flamePos);

		scene->add(cubemapFlame);
	}

	scene->camera->translate(glm::vec3(0.0f, 0.0f, -3.0f));

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

std::string toString(int i)
{
	return std::to_string(static_cast<long long>(i));
}
