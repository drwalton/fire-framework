#include "Scene.hpp"
#include "Camera.hpp"
#include "Texture.hpp"
#include "Particles.hpp"
#include "Mesh.hpp"
#include "PRTMesh.hpp"
#include "SphereFunc.hpp"
#include "UserInput.hpp"

#include <glm.hpp>
#include <GL/glut.h>
#include <gtc/matrix_transform.hpp>

#include <fstream>

/* Light Demo
 * Demo for benchmarking different numbers of light sources.
 * Please note that Phong light performance is likely to depend upon the 
 *   maximum number of Phong lights, as defined in "GC.hpp" in
 *   "fire-framework-lib".
 */

int init();
void display();
void reshape (int, int);
void keyboard(unsigned char, int, int);

glm::mat4 randMat();

Mesh*    bunny;
PRTMesh* prtBunny;

Scene* scene;
SHLight* light;

bool rotateOne = true;
bool rotateAll = false;

const int k = 5;

int lightChoice;
int numLights;

std::vector<SHLight*> shLights;
std::vector<PhongLight*> phongLights;

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
	/* Ambient Light Properties */
	const float ambIntensity = 0.5f;
	const glm::vec4 ambColor(0.7f, 0.7f, 0.9f, 1.0f); //light blue

	/* Bunny Phong Properties */
	const float bunnySpecExp = 1.0f;

	glClearColor(ambColor.x, ambColor.y, ambColor.z, ambColor.w);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	scene = new Scene();
	scene->setAmbLight(ambColor * ambIntensity);

	std::cout << "Please choose lighting type:" << std::endl;
	std::cout << ">  1. Blinn-Phong Lighting" << std::endl;
	std::cout << ">  2. Diffuse PRT" << std::endl;

	lightChoice = UserInput::getInt(1, 2, "Please enter your choice:");

	if(lightChoice == 1) // Blinn-Phong
	{
		std::cout << "The current max number of lights is "<< 
			std::to_string(static_cast<long long>(GC::maxPhongLights)) <<
			std::endl;
		numLights = UserInput::getInt(0, GC::maxPhongLights, 
			"Please enter required no. of lights:");

		Texture* bunnyDiffTex = new Texture("stanfordDiff.png");
		Texture* bunnySpecTex = new Texture("stanfordSpec.png");

		LightShader* bunnyShader = new LightShader(false, "BlinnPhong");

		bunny = new Mesh("stanford.obj", 
			bunnyDiffTex, bunnyDiffTex, bunnySpecTex, 
			bunnySpecExp, bunnyShader);

		scene->add(bunny);

		for(int l = 0; l < numLights; ++l)
		{
			PhongLight* light = new PhongLight(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
			phongLights.push_back(light);
			scene->add(light);
		}
	}

	else if(lightChoice == 2) // SHLights
	{
		numLights = UserInput::getInt(0, 1000000, 
			"Please enter required no. of lights:");
		
		const std::string filename = "stanford.obj";
		const std::string diffTexture = "stanfordDiff.png";
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

		scene->add(prtBunny);

		for(int l = 0; l < numLights; ++l)
		{
			SHLight* light = new SHLight(
			[] (float theta, float phi) -> glm::vec3 
			{
				float val = pulse(theta, phi, glm::vec3(1.0f, 0.0f, 0.0f), 5.0f, 1.0f);
				return glm::vec3(val, val, val);
			});
			shLights.push_back(light);
			scene->add(light);
		}
	}

	scene->camera->translate(glm::vec3(0.0f, 0.0f, -7.0f));

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
	if(lightChoice == 1) // Phong lights
	{
		scene->phongManager.updateBlock();
	}
	if(lightChoice == 2)
	{
		if(rotateOne)
		{
			glm::mat4 randmat = randMat();
			SHMat rotation(glm::mat3(randMat()), GC::nSHBands);
			for(auto l = shLights.begin(); l != shLights.end(); ++l)
			{
				(*l)->rotateCoeffts(rotation);
			}
		}

		if(rotateAll)
		{
			for(auto l = shLights.begin(); l != shLights.end(); ++l)
			{
				(*l)->rotateCoeffts(randMat());
			}
		}
	}
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

glm::mat4 randMat()
{
	float theta = randf(-PI / 2, PI / 2), phi = randf(0.0f, 2 * PI);
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0), phi, glm::vec3(1.0, 0.0, 0.0));
	rotation = glm::rotate(rotation,     theta, glm::vec3(0.0, 1.0, 0.0));
	return rotation;
}
