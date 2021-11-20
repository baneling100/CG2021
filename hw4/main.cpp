#include <math.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "light.h"
#include "model.h"

static unsigned int width = 1000;
static unsigned int height = 1000;

static bool mouseRotatePressed = false;
static bool mouseZoomPressed = false;

static glm::vec3 axisZ = glm::vec3(0.f, 0.5f, 0.866025f); // orientation
static glm::vec3 axisY = glm::vec3(0.f, 0.866025f, -0.5f); // view-up vector
static glm::vec3 origin = glm::vec3(0.f, 0.f, 0.f);
static float dist = 1000.f;
static float fovy = 45.f;
static int	 lastX = 0, lastY = 0;
static float angle_speed = 0.003f, speed = 10.f;
static int freq = 100;

static bool fullScreen = false;

PokeBall pokeball;
UltraBall ultraball;
NetBall netball;
TimerBall timerball;
MasterBall masterball;
Table table;

Model::Points opaque_points, trans_points;
Model::Faces opaque_faces, trans_faces;

class FaceComparator {
  public:
	FaceComparator(glm::vec3 &camera) : camera(camera) {};

	bool operator()(const std::tuple<int, int, int, Material *> &a,
					const std::tuple<int, int, int, Material *> &b) const {
						const auto &[a1, a2, a3, am] = a;
						const auto &[b1, b2, b3, bm] = b;
						return std::min(std::min(glm::distance(trans_points[a1].first, camera),
												 glm::distance(trans_points[a2].first, camera)),
												 glm::distance(trans_points[a3].first, camera)) >
							   std::min(std::min(glm::distance(trans_points[b1].first, camera),
												 glm::distance(trans_points[b2].first, camera)),
												 glm::distance(trans_points[b3].first, camera));
					}
  private:
	glm::vec3 camera;
};

void reshape(int w, int h)
{
	width = glutGet(GLUT_WINDOW_WIDTH);
	height = glutGet(GLUT_WINDOW_HEIGHT);

	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy /*field of view angle*/,
				   (double)w / h,
				   0.1 /*near clipping plane*/,
				   10000.0 /* far clipping plane */);
	
	glm::vec3 camera = origin + dist * axisZ;

	gluLookAt(camera.x, camera.y, camera.z, origin.x, origin.y, origin.z, axisY.x, axisY.y, axisY.z);
	glMatrixMode(GL_MODELVIEW);

	static int cnt = 0;
	if (!cnt) std::sort(trans_faces.begin(), trans_faces.end(), FaceComparator(camera));
	cnt = (cnt + 1) % freq;
}

void draw(Model::Points &points, Model::Faces &faces)
{	
	for (auto &[v1, v2, v3, m] : faces) {
		m->set();
		glBegin(GL_TRIANGLES);
		glNormal3f(points[v1].second.x, points[v1].second.y, points[v1].second.z);
		glVertex3f(points[v1].first.x, points[v1].first.y, points[v1].first.z);
		glNormal3f(points[v2].second.x, points[v2].second.y, points[v2].second.z);
		glVertex3f(points[v2].first.x, points[v2].first.y, points[v2].first.z);
		glNormal3f(points[v3].second.x, points[v3].second.y, points[v3].second.z);
		glVertex3f(points[v3].first.x, points[v3].first.y, points[v3].first.z);
		glEnd();
	}
}

void display()
{
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw(opaque_points, opaque_faces);
	draw(trans_points, trans_faces);
	glFlush();
	glutSwapBuffers();
}

void keyboardCB(unsigned char keyPressed, int x, int y)
{
	switch (keyPressed) {
	case 'f':
		if (fullScreen == true) {
			glutReshapeWindow(width, height);
			fullScreen = false;
		} else {
			glutFullScreen();
			fullScreen = true;
		}
		break;
	case 'e':
		dist -= speed;
		if (dist < 0.1) dist = 0.1;
		reshape(width, height);
		break;
	case 'r':
		dist += speed;
		reshape(width, height);
		break;
	case 'w':
		origin += speed * axisY;
		reshape(width, height);
		break;
	case 's':
		origin -= speed * axisY;
		reshape(width, height);
		break;
	case 'a':
		{
			glm::vec3 axisX = glm::cross(axisY, axisZ);
			origin -= speed * axisX;
		}
		reshape(width, height);
		break;
	case 'd':
		{
			glm::vec3 axisX = glm::cross(axisY, axisZ);
			origin += speed * axisX;
		}
		reshape(width, height);
		break;
	case 'q':
		exit(0);
		break;
	}
	glutPostRedisplay();
}

void mouseCB(int button, int state, int x, int y)
{
	if (state == GLUT_UP) {
		mouseRotatePressed = false;
		mouseZoomPressed   = false;
	}
	else if (button == GLUT_LEFT_BUTTON && GLUT_ACTIVE_CTRL == glutGetModifiers()) {
		// do something herer
		lastY = y;

		// std::cout << "zoom click" << std::endl;

		mouseRotatePressed = false;
		mouseZoomPressed   = true;
	}
	else if (button == GLUT_LEFT_BUTTON) {
		// do something here
		lastX = x;
		lastY = y;

		//std::cout << "rotate click" << std::endl;

		mouseRotatePressed = true;
		mouseZoomPressed   = false;
	}
	glutPostRedisplay();
}

void motionCB(int x, int y)
{
	if (mouseRotatePressed == true)
	{
		// do something here
		int winSize = std::min(width, height);
		double r2 = winSize * winSize / 4.0;
		double cx = width / 2.0;
		double cy = height / 2.0;
		
		double dx = x - cx;
		double dy = cy - y;
		double lastDx = lastX - cx;
		double lastDy = cy - lastY;

		double d1 = dx * dx + dy * dy;
		double d2 = lastDx * lastDx + lastDy * lastDy;
		int    d3 = (x - lastX) * (x - lastX) + (y - lastY) * (y - lastY);
		if (d1 <= r2 && d2 <= r2 && d3) {
			glm::vec3 axisX = glm::cross(axisY, axisZ);

			double dz = sqrt(r2 - d1);
			double lastDz = sqrt(r2 - d2);

			glm::vec3 s1 = glm::mat3(axisX, axisY, axisZ) * glm::vec3(dx, dy, dz);
			glm::vec3 s2 = glm::mat3(axisX, axisY, axisZ) * glm::vec3(lastDx, lastDy, lastDz);
			glm::vec3 axis = glm::cross(s1, s2);
			float theta = angle_speed * sqrt(d3);
			
			glm::quat rotate = glm::rotate(glm::quat(1.f, 0.f, 0.f, 0.f), theta, axis);
			axisZ = glm::normalize(rotate * axisZ);
			glm::vec3 temp = rotate * axisY;
			axisY = glm::normalize(temp - glm::dot(temp, axisZ) * axisZ);
			
			reshape(width, height);
		}

		lastX = x;
		lastY = y;

		//std::cout << "rotate drag" << std::endl;
	}
	else if (mouseZoomPressed == true)
	{
		// do something here
		fovy += (y - lastY) * 0.1;
		if (fovy < 0.1) fovy = 0.1;
		else if (fovy > 179) fovy = 179;

		reshape(width, height);

		lastY = y;

		//std::cout << "zoom drag" << std::endl;
	}
	glutPostRedisplay();
}

void idle() { glutPostRedisplay(); }

int main(int argc, char** argv)
{
	pokeball.parse().collect(opaque_points, opaque_faces, trans_points, trans_faces);
	ultraball.parse().collect(opaque_points, opaque_faces, trans_points, trans_faces);
	netball.parse().collect(opaque_points, opaque_faces, trans_points, trans_faces);
	timerball.parse().collect(opaque_points, opaque_faces, trans_points, trans_faces);
	masterball.parse().collect(opaque_points, opaque_faces, trans_points, trans_faces);
	table.parse().collect(opaque_points, opaque_faces, trans_points, trans_faces);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(width, height);
	glutCreateWindow("Pokeballs");

	glEnable(GL_LIGHTING);
	light0.set();
	light1.set();
	light2.set();

	glShadeModel(GL_SMOOTH);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	reshape(width, height);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glClearDepth(1.0f);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboardCB);
	glutReshapeFunc(reshape);
	glutMotionFunc(motionCB);
	glutMouseFunc(mouseCB);

	glutMainLoop();
	return 0;
}

