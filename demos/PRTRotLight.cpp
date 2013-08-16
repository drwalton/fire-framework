#include "Scene.hpp"
#include "Camera.hpp"
#include "Texture.hpp"
#include "PRTMesh.hpp"
#include "SH.hpp"
#include "SHMat.hpp"
#include "Light.hpp"
#include "SphereFunc.hpp"

#include <glm.hpp>
#include <GL/glut.h>
#include <gtc/matrix_transform.hpp>
#include <fstream>

/* PRT Light Rotation Demo
 * Displays an object illuminated using diffuse PRT.
 * Environment is composed of a single SH light source
 *   which may be rotated by the user using the "tfgh" keys.
 * Camera controlled by "wasd" and "ijkl" keys, with "c" switching
 *   between "centered rotation" and "free view" modes.
 */

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
    glutCreateWindow("SH Lighting Demo");
    glewInit();
    int good = init();
    if(!good) return 0;
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
    glutMainLoop();

	delete scene;
}

int init()
{
	glClearColor(0.8f, 0.8f, 1.0f, 1.0f); // Light blue

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	scene = new Scene();

	/* Edit mesh filename and PRT type here. */
	/* mode should be UNSHADOWED, SHADOWED or INTERREFLECTED */
	const std::string filename = "torii.obj";
	const PRTMode mode = SHADOWED;

	/* Check if baked file exists. If not, make one. */
	const std::string bakedFilename = filename + "prt" + 
		(mode == UNSHADOWED ? "u" :
			mode == SHADOWED ? "s" : "i")
		+ "5";
	std::ifstream temp(bakedFilename);
	if(!temp)
		PRTMesh::bake(INTERREFLECTED, "torii.obj", "greenWhite.png", 40, 5, 1);

	SHShader* shShader = new SHShader(false, "diffPRT");
	PRTMesh* torii = new PRTMesh("torii.obj.prti5", shShader);
	scene->add(torii);

	/* A simple SH light source consisting of a pulse in the +ve x direction */
	light = new SHLight(
		[] (float theta, float phi) -> glm::vec3 
		{
			float val = pulse(theta, phi, glm::vec3(1.0f, 0.0f, 0.0f), 4.0f, 3.0f);
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

	/* Rotate SH light to theta, phi */
	glm::mat4 rotation(1.0f);
	rotation = glm::rotate(glm::mat4(1.0), phi, glm::vec3(1.0, 0.0, 0.0));
	rotation = glm::rotate(rotation, theta, glm::vec3(0.0, 1.0, 0.0));
	light->rotateCoeffts(rotation);

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

	/* Rotate the SH Light */
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
