#include <cmath>
#include <string>
#include <filesystem>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "light.h"
#include "model.h"
#include "export.h"

static unsigned int width = 1280, height = 720;
static bool mouseRotatePressed = false, mouseZoomPressed = false, fullScreen = false;
static float dist = 10.0f, fovy = 45.0f, angle_speed = 0.003f, speed = 0.1f;
static int lastX = 0, lastY = 0, freq = 32, num_threads = 1;
static glm::vec3 axisZ = glm::vec3(0.0f, 0.0f, 1.0f); // orientation
static glm::vec3 axisY = glm::vec3(0.0f, 1.0f, 0.0f); // view-up vector
static glm::vec3 origin = glm::vec3(0.0f, 0.0f, 0.0f);

Light *lights[] = {new Light(GL_LIGHT0, (GLfloat[4]){1.0f, 1.0f, 1.0f, 1.0f},
										(GLfloat[4]){1.0f, 1.0f, 1.0f, 1.0f},
										(GLfloat[4]){1.0f, 1.0f, 1.0f, 1.0f},
										(GLfloat[4]){-1.0f, 1.0f, 1.0f, 0.0f}),
				   new Light(GL_LIGHT1, (GLfloat[4]){1.0f, 1.0f, 1.0f, 1.0f},
										(GLfloat[4]){1.0f, 1.0f, 1.0f, 1.0f},
										(GLfloat[4]){1.0f, 1.0f, 1.0f, 1.0f},
										(GLfloat[4]){-1.0f, 1.0f, 1.0f, 0.0f}),
				   new Light(GL_LIGHT2, (GLfloat[4]){1.0f, 1.0f, 1.0f, 1.0f},
										(GLfloat[4]){1.0f, 1.0f, 1.0f, 1.0f},
										(GLfloat[4]){1.0f, 1.0f, 1.0f, 1.0f},
										(GLfloat[4]){-1.0f, 1.0f, 1.0f, 0.0f})};
Model *models[] = {new Background(), new WoodenSphere(), new Dice(), new MirrorSphere(), new Table(),
				   new PokeBall(), new UltraBall(), new NetBall(), new TimerBall(), new MasterBall()};
Points points; Faces opaque_faces, trans_faces; Images images;

class FaceCompare {
  public:
	FaceCompare(glm::vec3 &camera) : camera(camera) {};

	bool operator()(const Face &a,
					const Face &b) const {
						const auto &[af1, af2, af3, am, at] = a;
						const auto &[bf1, bf2, bf3, bm, bt] = b;
						const auto &[ap1, an1, at1] = points[af1];
						const auto &[ap2, an2, at2] = points[af2];
						const auto &[ap3, an3, at3] = points[af3];
						const auto &[bp1, bn1, bt1] = points[bf1];
						const auto &[bp2, bn2, bt2] = points[bf2];
						const auto &[bp3, bn3, bt3] = points[bf3];
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
	for (int i = 0; i < 10; i++)
		models[i]->parse().collect(points, opaque_faces, trans_faces, images);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); glClearDepth(1.0);
	glShadeModel(GL_SMOOTH);
	for (int i = 0; i < 3; i++) lights[i]->set();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
}

void draw(Faces &faces)
{
	static unsigned int texture = 0;
	static Material *material = nullptr;

	glBegin(GL_TRIANGLES);
	for (auto &[p1, p2, p3, m, t] : faces) {
		if (material != m) {
			glEnd();
			if (!material) glEnable(GL_LIGHTING);
			else if (!m) glDisable(GL_LIGHTING);
			if (m) m->set();
			material = m;
			glBegin(GL_TRIANGLES);
		}
		if (texture != t) {
			glEnd();
			glBindTexture(GL_TEXTURE_2D, t);
			texture = t;
			glBegin(GL_TRIANGLES);
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

	// draw faces
	draw(opaque_faces);
	draw(trans_faces);

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
	gluPerspective(fovy, (float)width / height, 0.001f, 30.0f);
	gluLookAt(camera.x, camera.y, camera.z, origin.x, origin.y, origin.z, axisY.x, axisY.y, axisY.z);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// depth ordering
	static int cnt = 0;
	if (!cnt) std::sort(trans_faces.begin(), trans_faces.end(), FaceCompare(camera));
	cnt = (cnt + 1) % freq;
}

std::string get_filename(std::string &&pred)
{
	static int cnt = 0;
	std::string filename;
	while (std::filesystem::exists(filename = (pred + std::to_string(cnt) + ".bmp"))) cnt++;
	return filename;
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
		if (dist < 0.001f) dist = 0.001f;
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
	case 'z': // opengl
	{
		auto filename = get_filename("images/opengl_");
		export_opengl_image(filename, width, height);
	}
		break;
	case 'x': // normal mode
	{
		auto filename = get_filename("images/raytraced_");
		glm::vec3 camera =  origin + dist * axisZ;
		glm::vec3 axisX = glm::cross(axisY, axisZ);
		export_raytraced_image(filename, width, height, fovy, camera, origin, axisX,
							   axisY, lights, models, images, num_threads, DEFAULT);
	}
		break;
	case 'c': // soft shadows
	{
		auto filename = get_filename("images/soft_shadows_");
		glm::vec3 camera = origin + dist * axisZ;
		glm::vec3 axisX = glm::cross(axisY, axisZ);
		export_raytraced_image(filename, width, height, fovy, camera, origin, axisX,
							   axisY, lights, models, images, num_threads, SOFT_SHADOWS);
	}
		break;
	case 'v': // depth of field
	{
		auto filename = get_filename("images/depth_of_field_");
		glm::vec3 camera = origin + dist * axisZ;
		glm::vec3 axisX = glm::cross(axisY, axisZ);
		export_raytraced_image(filename, width, height, fovy, camera, origin, axisX,
							   axisY, lights, models, images, num_threads, DEPTH_OF_FIELD);
	}
		break;
	case 'b': // motion blur
	{
		auto filename = get_filename("images/motion_blur_");
		glm::vec3 camera = origin + dist * axisZ;
		glm::vec3 axisX = glm::cross(axisY, axisZ);
		export_raytraced_image(filename, width, height, fovy, camera, origin, axisX,
							   axisY, lights, models, images, num_threads, MOTION_BLUR);
	}
		break;
	case 'n': // bump mapping
	{
		auto filename = get_filename("images/bump_mapping_");
		glm::vec3 camera = origin + dist * axisZ;
		glm::vec3 axisX = glm::cross(axisY, axisZ);
		export_raytraced_image(filename, width, height, fovy, camera, origin, axisX,
							   axisY, lights, models, images, num_threads, BUMP_MAPPING);
	}
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
	if (argc >= 2) num_threads = atoi(argv[1]);

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
