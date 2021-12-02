#include <math.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "light.h"
#include "model.h"

static unsigned int width = 1280, height = 720;
static bool mouseRotatePressed = false, mouseZoomPressed = false, fullScreen = false;
static float dist = 1000.0f, fovy = 45.0f, angle_speed = 0.003f, speed = 10.0f;
static int lastX = 0, lastY = 0, freq = 32;
static glm::vec3 axisZ = glm::vec3(0.0f, 0.0f, 1.0f); // orientation
static glm::vec3 axisY = glm::vec3(0.0f, 1.0f, 0.0f); // view-up vector
static glm::vec3 origin = glm::vec3(0.0f, 0.0f, 0.0f);

Light light = Light(GL_LIGHT0, (GLfloat[4]){1.0f, 1.0f, 1.0f, 1.0f},
							   (GLfloat[4]){1.0f, 1.0f, 1.0f, 1.0f},
							   (GLfloat[4]){1.0f, 1.0f, 1.0f, 1.0f},
							   (GLfloat[4]){-1.0f, 0.3f, 1.0f, 0.0f});

PokeBall pokeball; UltraBall ultraball; NetBall netball;
TimerBall timerball; MasterBall masterball; Table table;
Points opaque_points, trans_points, background_points;
Faces opaque_faces, trans_faces, background_faces;
GlassSphere glass_sphere; WoodenSphere wooden_sphere; Dice dice; Background background;

class TransFaceCompare {
  public:
	TransFaceCompare(glm::vec3 &camera) : camera(camera) {};

	bool operator()(const std::tuple<int, int, int, Material *, unsigned int> &a,
					const std::tuple<int, int, int, Material *, unsigned int> &b) const {
						const auto &[af1, af2, af3, am, at] = a;
						const auto &[bf1, bf2, bf3, bm, bt] = b;
						const auto &[ap1, an1, at1] = trans_points[af1];
						const auto &[ap2, an2, at2] = trans_points[af2];
						const auto &[ap3, an3, at3] = trans_points[af3];
						const auto &[bp1, bn1, bt1] = trans_points[bf1];
						const auto &[bp2, bn2, bt2] = trans_points[bf2];
						const auto &[bp3, bn3, bt3] = trans_points[bf3];
						return std::min(std::min(glm::distance(ap1, camera),
												 glm::distance(ap2, camera)),
												 glm::distance(ap3, camera)) >
							   std::min(std::min(glm::distance(bp1, camera),
												 glm::distance(bp2, camera)),
												 glm::distance(bp3, camera));
					}
  private:
	glm::vec3 camera;
};

void init()
{
	// preprocess
	background.parse().collect(background_points, background_faces, trans_points, trans_faces);
	wooden_sphere.parse().collect(opaque_points, opaque_faces, trans_points, trans_faces);
	glass_sphere.parse().collect(opaque_points, opaque_faces, trans_points, trans_faces);
	dice.parse().collect(opaque_points, opaque_faces, trans_points, trans_faces);
	pokeball.parse().collect(opaque_points, opaque_faces, trans_points, trans_faces);
	ultraball.parse().collect(opaque_points, opaque_faces, trans_points, trans_faces);
	netball.parse().collect(opaque_points, opaque_faces, trans_points, trans_faces);
	timerball.parse().collect(opaque_points, opaque_faces, trans_points, trans_faces);
	masterball.parse().collect(opaque_points, opaque_faces, trans_points, trans_faces);
	table.parse().collect(opaque_points, opaque_faces, trans_points, trans_faces);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); glClearDepth(1.0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	light.set();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
}

void draw(Points &points, Faces &faces)
{
	unsigned int texture = 0;
	Material *material = NULL;

	glBegin(GL_TRIANGLES);
	for (auto &[p1, p2, p3, m, t] : faces) {
		if (texture != t) {
			glEnd();
			glBindTexture(GL_TEXTURE_2D, t);
			texture = t;
			glBegin(GL_TRIANGLES);
		}
		if (material != m) {
			m->set();
			material = m;
		}
		auto &[v1, n1, t1] = points[p1];
		auto &[v2, n2, t2] = points[p2];
		auto &[v3, n3, t3] = points[p3];
		glNormal3f(n1.x, n1.y, n1.z); if (texture) glTexCoord2f(t1.x, t1.y); glVertex3f(v1.x, v1.y, v1.z);
		glNormal3f(n2.x, n2.y, n2.z); if (texture) glTexCoord2f(t2.x, t2.y); glVertex3f(v2.x, v2.y, v2.z);
		glNormal3f(n3.x, n3.y, n3.z); if (texture) glTexCoord2f(t3.x, t3.y); glVertex3f(v3.x, v3.y, v3.z);
	}
	glEnd();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// draw background
	glDisable(GL_LIGHTING);
	draw(background_points, background_faces);
	glEnable(GL_LIGHTING);

	// draw other opaque faces
	draw(opaque_points, opaque_faces);

	// draw translucent faces
	draw(trans_points, trans_faces);

	glFlush();
	glutSwapBuffers();
}

void reshape(int w, int h)
{
	width = w;
	height = h;
	glm::vec3 camera = origin + dist * axisZ;

	// view and camera
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (float)width / height, 0.1f, 3000.0f);
	gluLookAt(camera.x, camera.y, camera.z, origin.x, origin.y, origin.z, axisY.x, axisY.y, axisY.z);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// depth ordering
	static int cnt = 0;
	if (!cnt) std::sort(trans_faces.begin(), trans_faces.end(), TransFaceCompare(camera));
	cnt = (cnt + 1) % freq;
}

void keyboardCB(unsigned char keyPressed, int x, int y)
{
	switch (keyPressed) {
	case 'f': // fullscreen
		if (fullScreen == true) {
			glutReshapeWindow(width, height);
			fullScreen = false;
		} else {
			glutFullScreen();
			fullScreen = true;
		}
		break;
	case 'e': // dolly in
		dist -= speed;
		if (dist < 0.1f) dist = 0.1f;
		reshape(width, height);
		break;
	case 'r': // dolly out
		dist += speed;
		reshape(width, height);
		break;
	case 'w': // translate up
		origin += speed * axisY;
		reshape(width, height);
		break;
	case 's': // translate down
		origin -= speed * axisY;
		reshape(width, height);
		break;
	case 'a': // translate left
		{
			glm::vec3 axisX = glm::cross(axisY, axisZ);
			origin -= speed * axisX;
		}
		reshape(width, height);
		break;
	case 'd': // translate right
		{
			glm::vec3 axisX = glm::cross(axisY, axisZ);
			origin += speed * axisX;
		}
		reshape(width, height);
		break;
	case 'q': // quit
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
	else if (button == GLUT_LEFT_BUTTON && GLUT_ACTIVE_CTRL == glutGetModifiers()) { // zoom click
		lastY = y;
		mouseRotatePressed = false;
		mouseZoomPressed   = true;
	}
	else if (button == GLUT_LEFT_BUTTON) { // rotate click
		lastX = x;
		lastY = y;
		mouseRotatePressed = true;
		mouseZoomPressed   = false;
	}
	glutPostRedisplay();
}

void motionCB(int x, int y)
{
	if (mouseRotatePressed == true) { // rotate drag
		int winSize = std::min(width, height);
		float r2 = winSize * winSize / 4.0f;
		float cx = width / 2.0f, cy = height / 2.0f;
		float dx = x - cx, dy = cy - y, lastDx = lastX - cx, lastDy = cy - lastY;
		float d1 = dx * dx + dy * dy, d2 = lastDx * lastDx + lastDy * lastDy;
		int    d3 = (x - lastX) * (x - lastX) + (y - lastY) * (y - lastY);
		if (d1 <= r2 && d2 <= r2 && d3) {
			glm::vec3 axisX = glm::cross(axisY, axisZ);
			float dz = sqrt(r2 - d1), lastDz = sqrt(r2 - d2);
			glm::vec3 s1 = glm::mat3(axisX, axisY, axisZ) * glm::vec3(dx, dy, dz);
			glm::vec3 s2 = glm::mat3(axisX, axisY, axisZ) * glm::vec3(lastDx, lastDy, lastDz);
			glm::vec3 axis = glm::cross(s1, s2);
			float theta = angle_speed * sqrt(d3);
			glm::quat rotate = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), theta, axis);
			axisZ = glm::normalize(rotate * axisZ);
			glm::vec3 temp = rotate * axisY;
			axisY = glm::normalize(temp - glm::dot(temp, axisZ) * axisZ);
			reshape(width, height);
		}
		lastX = x; lastY = y;
	}
	else if (mouseZoomPressed == true) { // zoom drag
		fovy += (y - lastY) * 0.1f;
		if (fovy < 0.1f) fovy = 0.1f;
		else if (fovy > 179.0f) fovy = 179.0f;
		reshape(width, height);
		lastY = y;
	}
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(width, height);
	glutCreateWindow("Pokeballs");

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboardCB);
	glutMouseFunc(mouseCB);
	glutMotionFunc(motionCB);

	glutMainLoop();
	return 0;
}
