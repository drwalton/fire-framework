#include "Scene.hpp"
#include "Camera.hpp"
#include "Texture.hpp"
#include "Particles.hpp"
#include "Mesh.hpp"
#include "AOMesh.hpp"
#include "PRTMesh.hpp"
#include "SH.hpp"
#include "SHMat.hpp"
#include "SphereFunc.hpp"
#include "SpherePlot.hpp"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <GL/glut.h>

#include <iomanip>

/* SH Rotation Test Demo
 * This demo plots two SH functions. The one on the left is reprojected from a 
 *   rotated spherical function on each frame. The one on the right is having 
 *   its coefficients rotated by the author's implementation of Ivanic's SH 
 *   rotation. Both functions are recovered from SH coefficients and replotted
 *   after every rotation. The functions may be rotated using the "tfgh" keys.
 * If the SH rotation is working correctly, the two recovered functions should 
 *   appear identical. Note that the application will run slowly when rotating,
 *   due to the SH rotation, reprojection and (most importantly) plot mesh 
 *   generation performed. Also, the left-hand function will "wobble" due to 
 *   inaccuracies in the Monte Carlo projection method.
 * Camera controlled by "wasd" and "ijkl" keys, with "c" switching
 *   between "centered rotation" and "free view" modes.
 */

int init();
void display();
void reshape (int, int);
void keyboard(unsigned char, int, int);
void printInfo();
void rotate();

void addSHArray(Scene* scene, glm::vec3 pos, int nBands, float scale, float spacing);

const int nSwirls = 400;
const int nSparks = 5;

float theta = 0.0f;
float phi = 0.0f;
float psi = 0.0f;

Scene* scene;
SHLight* light;
SpherePlot* rotated;
SpherePlot* reProjected;
SpherePlot* shRotated;
std::vector<glm::vec3> proj;
std::vector<glm::vec3> reProj;
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

	reProj = proj; rotProj = proj;

	rotated = new SpherePlot(
		[] (float theta, float phi) -> 
		float {return SH::evaluate(proj, theta, phi).x;},
		40, plotShader);

	reProjected = new SpherePlot(
		[] (float theta, float phi) -> 
		float {return SH::evaluate(reProj, theta, phi).x;},
		40, plotShader);

	shRotated = new SpherePlot(
		[] (float theta, float phi) -> 
		float {return SH::evaluate(rotProj, theta, phi).x;},
		40, plotShader);
	
	rotated->translate(glm::vec3(0.0f, 0.0f, 1.5f));
	reProjected->translate(glm::vec3(-1.5f, 0.0f, 0.0f));
	shRotated->translate(glm::vec3(1.5f, 0.0f, 0.0f));

	scene->add(rotated); scene->add(shRotated); scene->add(reProjected);

	return 1;
}

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
		theta += 3.6f;
		rotate();
		break;
	case 'f':
		theta -= 3.6f;
		rotate();
		break;
	case 't':
		phi -= 3.6f;
		rotate();
		break;
	case 'g':
		phi += 3.6f;
		rotate();
		break;
	case 'y':
		psi -= 3.6f;
		rotate();
		break;
	case 'h':
		psi += 3.6f;
		rotate();
		break;
	case 'p':
		printInfo();
		rotate();
		break;
    case 27:
        exit(0);
        return;
    }
}

void printInfo()
{
	std::cout << std::fixed << std::setprecision(2) << std::setw(5);
	std::cout << "Projection of rotation:\n";
	for(auto i = proj.begin(); i != proj.end(); ++i)
		std::cout << i->x << " ";
	std::cout << "\n";
	std::cout << "SH rotated projection:\n";
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

void rotate()
{
	rotation = glm::rotate(glm::mat4(1.0f), psi, glm::vec3(0.0f, 0.0f, 1.0f));
	rotation = glm::rotate(rotation, phi, glm::vec3(1.0f, 0.0f, 0.0f));
	rotation = glm::rotate(rotation, theta, glm::vec3(0.0f, 1.0f, 0.0f));


	reProj = SH::shProject(20, GC::nSHBands, 
	[] (float theta, float phi) -> glm::vec3 
	{
		return glm::vec3(pulse(theta, phi, 
			glm::vec3(rotation * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)),
			4.0f, 1.0f));
	}
	);

	glm::mat4 rotation2 = glm::rotate(glm::mat4(1.0f), -phi, glm::vec3(1.0f, 0.0f, 0.0f));
	rotation2 = glm::rotate(rotation2, -theta, glm::vec3(0.0f, 1.0f, 0.0f));

	shRotation = SHMat(rotation, GC::nSHBands);

	std::cout << "Original matrix: " << std::endl;
	Matrix<float>::print(glm::mat3(rotation));
	std::cout << "Other matrix: " << std::endl;
	Matrix<float>::print(glm::mat3(rotation2));

	rotProj = shRotation * proj;

	rotated->setRotation(rotation);

	reProjected->replot(
		[] (float theta, float phi) -> 
		float {return SH::evaluate(reProj, theta, phi).x;}, 40);
	
	shRotated->replot(		
		[] (float theta, float phi) -> 
		float {return SH::evaluate(rotProj, theta, phi).x;}, 40);

	display();
}
