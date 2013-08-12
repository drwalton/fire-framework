#include "Scene.hpp"
#include "Texture.hpp"
#include "Particles.hpp"
#include "Mesh.hpp"
#include "AOMesh.hpp"
#include "PRTMesh.hpp"
#include "SH.hpp"
#include "SHMat.hpp"
#include "SphereFunc.hpp"

#include <glm.hpp>
#include <GL/glut.h>

/* This file will contain the construction and rendering of the scene
 * I am working on right now. 
 */

int init();
void display();
void reshape (int, int);
void keyboard(unsigned char, int, int);
void printInfo();

void addSHArray(Scene* scene, glm::vec3 pos, int nBands, float scale, float spacing);

const int nSwirls = 400;
const int nSparks = 5;

float theta = 0.0f;
float phi = 0.0f;

Scene* scene;
SHLight* light;
SpherePlot* worldRotated;
SpherePlot* shRotated;
std::vector<glm::vec3> proj;
std::vector<glm::vec3> rotProj;
glm::mat4 rotation;
SHMat shRotation(GC::nSHBands);

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

int init()
{
	glClearColor(0.8f, 0.8f, 1.0f, 1.0f);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	scene = new Scene();

	Shader* plotShader = new Shader(false, "SpherePlot");

	proj = SH::shProject(20, GC::nSHBands, 
	[] (float theta, float phi) -> glm::vec3 
	{
		return glm::vec3(pulse(theta, phi, glm::vec3(1.0f, 0.0f, 0.0f), 4.0f, 1.0f));
	}
	);

	worldRotated = new SpherePlot(
		[] (float theta, float phi) -> 
		float {return SH::evaluate(proj, theta, phi).x;},
		50, plotShader);

	shRotated = new SpherePlot(
		[] (float theta, float phi) -> 
		float {return SH::evaluate(proj, theta, phi).x;},
		50, plotShader);

	worldRotated->translate(glm::vec3(-1.5f, 0.0f, 0.0f));
	shRotated->translate(glm::vec3(1.5f, 0.0f, 0.0f));

	scene->add(shRotated); scene->add(worldRotated);

	return 1;
}

void display()
{
	deTime = glutGet(GLUT_ELAPSED_TIME) - eTime;
	eTime = glutGet(GLUT_ELAPSED_TIME);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	scene->update(deTime);

	rotation = glm::mat4(1.0f);
	rotation = glm::rotate(glm::mat4(1.0), phi, glm::vec3(1.0, 0.0, 0.0));
	rotation = glm::rotate(rotation, theta, glm::vec3(0.0, 1.0, 0.0));

	worldRotated->setModelToWorld(rotation);
	worldRotated->translate(glm::vec3(-1.5f, 0.0f, 0.0f));

	shRotation = SHMat(rotation, GC::nSHBands);

	rotProj = shRotation * proj;

	shRotated->replot(		
		[] (float theta, float phi) -> 
		float {return SH::evaluate(rotProj, theta, phi).x;}, 40);

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
	case 'p':
		printInfo();
		break;
    case 27:
        exit(0);
        return;
    }
}

void printInfo()
{
	std::cout << "Original Projection:\n";
	for(auto i = proj.begin(); i != proj.end(); ++i)
		std::cout << i->x << " ";
	std::cout << "\n";
	std::cout << "Rotated projection:\n";
	for(auto i = rotProj.begin(); i != rotProj.end(); ++i)
		std::cout << i->x << " ";
	std::cout << "\n";
	std::cout << "Rotation Matrix:\n";
	for(int i = 0; i < 3; ++i)
	{
		for(int j = 0; j < 3; ++j)
			std::cout << rotation[j][i] << " ";
		std::cout << "\n";
	}
	std::cout << "SH Rotation Matrix:\n";
	shRotation.print();
}
