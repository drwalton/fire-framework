#include "Scene.hpp"
#include "Camera.hpp"
#include "SH.hpp"
#include "SphereFunc.hpp"
#include "SpherePlot.hpp"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <GL/glut.h>

/* SH Plot Demo
 * Demo illustrating plotting of SH approximations to functions, as well
 *   as displaying the first bands of the SH basis. 
 * Camera controlled by "wasd" and "ijkl" keys, with "c" switching
 *   between "centered rotation" and "free view" modes.
 */

template <typename Fn>
void plotApproximations(Fn f, int nBands, float spacing, glm::vec3 translate)
{
	Shader* plotShader = new Shader(false, "SpherePlot");

	SpherePlot* original = new SpherePlot(		
	[&f] (float theta, float phi) -> float 
	{
		return f(theta, phi);
	}
	, 100, plotShader);

	scene->add(original);

	for(int i = 1; i < nBands; ++i)
	{
		std::vector<glm::vec3> proj = SH::shProject(20, i, 
		[&f] (float theta, float phi) -> glm::vec3 
		{
			return glm::vec3(f(theta, phi));
		}
		);

		SpherePlot* recovered = new SpherePlot(		
		[&proj] (float theta, float phi) -> float 
		{
			return SH::evaluate(proj, theta, phi).x;
		}
		, 50, plotShader);

		recovered->translate(glm::vec3(i * spacing, 0.0, 0.0));
		scene->add(recovered);
	}
}

int init();
void display();
void reshape (int, int);
void keyboard(unsigned char, int, int);

void addSHArray(Scene* scene, glm::vec3 pos, int nBands, float scale, float spacing);

float theta = 0.0f;
float phi = 0.0f;

Scene* scene;
SpherePlot* plot;

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
	glClearColor(1.0, 1.0, 1.0, 1.0);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	scene = new Scene();

	std::cout << "Please choose demo:" << std::endl;
	std::cout << ">  1. Plot of SH Basis functions" << std::endl;
	std::cout << ">  2. Approximations to \"shell\" function" << std::endl;
	std::cout << ">  3. Approximations to \"apple\" function" << std::endl;
	std::cout << ">  4. Approximations to \"pulse\" function" << std::endl;
	std::cout << ">  5. Approximations to \"patches\" function" << std::endl;
	std::cout << ">  6. Approximations to \"swirls\" function" << std::endl;

	int choice = -1;
	while(!(
		choice == 1 || 
		choice == 2 || 
		choice == 3 || 
		choice == 4 || 
		choice == 5 || 
		choice == 6))
	{
		std::cout << std::endl;
		std::cout << "Please enter your choice: ";
		std::cin >> choice;
	}
	
	if(choice == 1) // SH Basis
		addSHArray(scene, glm::vec3(0.0f, -3.5, 0.0f), 7, 1.0f, 2.0f);

	else if(choice == 2) // Shell
		plotApproximations( 
		[] (float theta, float phi) -> float
		{
			return phi / (2 * PI);
		},
			8, 1.5f, glm::vec3(-7.0, 0.0, 0.0));

	else if(choice == 3) // Apple
		plotApproximations( 
		[] (float theta, float phi) -> float
		{
			return theta / PI;
		},
			8, 1.5f, glm::vec3(-7.0, 0.0, 0.0));

	else if(choice == 4) // Pulse
		plotApproximations( 
		[] (float theta, float phi) -> float
		{
			return pulse(theta, phi, glm::vec3(0.0f, 1.0f, 0.0f), 4.0f, 3.0f);
		},
			8, 2.0f, glm::vec3(-7.0, 0.0, 0.0));

	else if(choice == 5) // Patches
		plotApproximations( 
		[] (float theta, float phi) -> float
		{
			return patches(theta, phi);
		},
			8, 3.0f, glm::vec3(-7.0, 0.0, 0.0));

	else if(choice == 6) // Swirls
		plotApproximations( 
		[] (float theta, float phi) -> float
		{
			return swirls(theta, phi);
		},
			8, 3.0f, glm::vec3(-7.0, 0.0, 0.0));

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

// Renders an array of SH basis functions.
void addSHArray(Scene* scene, glm::vec3 pos, int nBands,
	float scale, float spacing)
{
	Shader* plotShader = new Shader(false, "SpherePlot");

	for(int l = 0; l < nBands; ++l)
		for(int m = -l; m <= l; ++m)
		{
			SpherePlot* plot = new SpherePlot(		
				[l, m] (float theta, float phi) -> float 
				{
					//float val = 0.2f;
					float val = SH::realSH(l, m, theta, phi);

					return val;
				}
				, 50, plotShader);
			plot->uniformScale(scale);
			plot->rotate(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
			plot->translate(pos + glm::vec3(m * spacing, l * spacing, 0.0f));
			
			scene->add(plot);
		}
}
