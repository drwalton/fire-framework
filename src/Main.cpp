#include "Scene.hpp"

#include <glm.hpp>

/* This file will contain the construction and rendering of the scene
 * I am working on right now. 
 */

int init();
void display();
void reshape (int, int);
void keyboard(unsigned char, int, int);
void updatePos();

const int nSwirls = 100;
const int nSparks = 5;

Scene* scene;
LightShader* shader;
AdvectParticles<nSwirls>* swirl;
AdvectParticles<nSwirls>* swirl2;
AdvectParticles<nSwirls>* swirl3;
AdvectParticles<nSparks>* sparks;
DirLight* d;
PointLight* p;
const int k = 5;

int time;
int dTime;

float z;
const float delta = 0.4f;

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	time = glutGet(GLUT_ELAPSED_TIME);
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

	delete shader;
	delete scene;
}

// Called by glutInit().
int init()
{
	z = -8.4f;
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	scene = new Scene();

	shader = new LightShader(false, "Simple", true, true, true);
	ArrSolid<36>* cube = Solid::Cube(shader);

	//scene->add(new DirLight(glm::vec3(0.0, 0.5, -1.0), 0.2));
	scene->add(new PointLight(glm::vec4(2.0, 2.0, 0.0, 1.0), 1.0));
	scene->add(new PointLight(glm::vec4(-2.0, -2.0, 0.0, 1.0), 1.0));
	scene->add(new PointLight(glm::vec4(2.0, 2.0, 0.0, 1.0), 1.0));

	for(int i = -k; i <= k; ++i)
		for(int j = -k; j <= k; ++j)
		{

			cube = Solid::Cube(shader);

			cube->concatTransform(glm::mat4(
				1.0, 0.0, 0.0, 0.0,
				0.0, 1.0, 0.0, 0.0,
				0.0, 0.0, 1.0, 0.0,
				3.0*i, 3.0*j, 0.0, 1.0));
			scene->add(cube);
		}

	scene->setAmbLight(0.01f);
	/*
	shader = new Shader(true, "Particles", false, false, false);
	swirl = new AdvectParticles<nSwirls>(shader, "bbFlame.png", "decay2.png");
	//scene->add(swirl);
	shader = new Shader(true, "Particles", false, false, false);
	swirl2 = new AdvectParticles<nSwirls>(shader, "bbFlame.png", "decay2.png");
	scene->add(swirl2);
	shader = new Shader(true, "Particles", false, false, false);
	swirl3 = new AdvectParticles<nSwirls>(shader, "bbFlame2.png", "decay2.png");
	scene->add(swirl3);
	shader = new Shader(true, "Sparks", false, false, false);
	sparks = new AdvectParticles<nSparks>(shader, "bbSolid.png", "sparkDecay.png", 
		1000, 200, glm::vec4(0.0, -0.00000002, 0.0, 0.0), glm::vec4(0.0, 0.00002, 0.0, 0.0),
		10, 0.00002f, 0.0f, 0.0f, 0.0005f, 0.0005f, true, true);
	scene->add(sparks);
	*/
	return 1;
}

// Perform rendering and updates here.
void display()
{
	dTime = glutGet(GLUT_ELAPSED_TIME) - time;
	time = glutGet(GLUT_ELAPSED_TIME);
	glClear(GL_COLOR_BUFFER_BIT);
	scene->update(dTime);
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
    switch (key)
    {
    	case 'w':
    		z += delta; 
			updatePos();
			break;
    	case 's':
    		z -= delta; 
			updatePos();
			break;
		case 't':
			d->on = !(d->on);
			scene->updateLight(d);
			break;
		case 'y':
			p->on = !(p->on);
			scene->updateLight(p);
			break;
      	case 27:
            exit(0);
            return;
    }
	std::cout<< z << std::endl;
}

void updatePos()
{
	scene->camera->setPos(glm::mat4(
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, z, 1.0));
}