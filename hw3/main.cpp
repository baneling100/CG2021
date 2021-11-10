
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/quaternion.hpp>

static unsigned int width = 1000;
static unsigned int height = 1000;

static bool mouseRotatePressed = false;
static bool mouseZoomPressed = false;

static glm::vec3 axisZ = glm::vec3(1.f, 0.f, 0.f); // orientation
static glm::vec3 axisY = glm::vec3(0.f, 1.f, 0.f); // view-up vector
static float dist = 400.f;
static float fovy = 45.f;
static int	 lastX = 0, lastY = 0;
static float speed = 0.003f;
static int red, green, blue;
static int rgbState; // 0: r -> g, 1: g -> b, 2: b -> r
static bool isPokeBall;

static bool fullScreen = false;

struct CrossSection {
	std::vector<glm::vec2> controlPoints; // X-Z plane
	float scalingFactor;
	glm::quat rotation;
	glm::vec3 position;

	CrossSection(std::vector<glm::vec2> &controlPoints, float scalingFactor,
				 glm::quat &rotation, glm::vec3 &position)
		: controlPoints(std::move(controlPoints)), scalingFactor(scalingFactor),
		  rotation(std::move(rotation)), position(std::move(position)) {}
};

static bool isBSpline;
static int numCrossSection, numControlPoints;
static std::vector<CrossSection> crossSections;

static int numStep = 16;

/**
 * @brief eat points on the curve 
 */
template<typename T>
T catmullRomSpline(float u, T p0, T p1, T p2, T p3)
{
	return (((     -p0 + 3.f * p1 - 3.f * p2 + p3)  * u +
			 (2.f * p0 - 5.f * p1 + 4.f * p2 - p3)) * u +
			 (     -p0            +       p2     )) * u / 2.f + p1;
}

/**
 * @brief eat control points which are not on the curve
 */
float BSpline(float v, float b0, float b1, float b2, float b3)
{
	return ((((      -b0 + 3.f * b1 - 3.f * b2 + b3)  * v +
			  ( 3.f * b0 - 6.f * b1 + 3.f * b2     )) * v +
			  (-3.f * b0            + 3.f * b2     )) * v +
			  (       b0 + 4.f * b1 +       b2     )) / 6.f;
}

void drawSomething()
{
	// Draw something Here
	red = 255;
	green = blue = rgbState = 0;

	std::vector<glm::vec3> lastPoints;
	bool first = true;

	for (int i = 1; i < numCrossSection; i++) // i - 1, i, i + 1, i + 2
		for (int j = 0; j < numStep; j++) {
			float u = (float)j / numStep;

			std::vector<glm::vec2> controlPoints;
			for (int k = 0; k < numControlPoints; k++) {
				controlPoints.emplace_back(
					catmullRomSpline(u, crossSections[i - 1].controlPoints[k].x,
										crossSections[i    ].controlPoints[k].x,
										crossSections[i + 1].controlPoints[k].x,
										crossSections[i + 2].controlPoints[k].x),
					catmullRomSpline(u, crossSections[i - 1].controlPoints[k].y,
										crossSections[i    ].controlPoints[k].y,
										crossSections[i + 1].controlPoints[k].y,
										crossSections[i + 2].controlPoints[k].y));
			}
			controlPoints.emplace(controlPoints.begin(), controlPoints[numControlPoints - 1]);
			controlPoints.emplace_back(controlPoints[1]);
			controlPoints.emplace_back(controlPoints[2]);

			float scalingFactor = catmullRomSpline(u, crossSections[i - 1].scalingFactor,
													  crossSections[i    ].scalingFactor,
													  crossSections[i + 1].scalingFactor,
													  crossSections[i + 2].scalingFactor);

			glm::quat rotation = glm::exp(
									catmullRomSpline(u, glm::log(crossSections[i - 1].rotation),
														glm::log(crossSections[i    ].rotation),
														glm::log(crossSections[i + 1].rotation),
														glm::log(crossSections[i + 2].rotation)));

			glm::vec3 position = glm::vec3(catmullRomSpline(u, crossSections[i - 1].position.x,
															   crossSections[i    ].position.x,
															   crossSections[i + 1].position.x,
															   crossSections[i + 2].position.x),
										   catmullRomSpline(u, crossSections[i - 1].position.y,
															   crossSections[i    ].position.y,
															   crossSections[i + 1].position.y,
															   crossSections[i + 2].position.y),
										   catmullRomSpline(u, crossSections[i - 1].position.z,
															   crossSections[i    ].position.z,
															   crossSections[i + 1].position.z,
															   crossSections[i + 2].position.z));
			std::vector<glm::vec3> points;
			for (int k = 1; k <= numControlPoints; k++)
				for (int l = 0; l < numStep; l++) {
					float v = (float)l / numStep;
					
					glm::vec3 local = isBSpline ?
									glm::vec3(BSpline(v, controlPoints[k - 1].x,
															controlPoints[k    ].x,
															controlPoints[k + 1].x,
															controlPoints[k + 2].x), 0.f,
												BSpline(v, controlPoints[k - 1].y,
															controlPoints[k    ].y,
															controlPoints[k + 1].y,
															controlPoints[k + 2].y)) :
									glm::vec3(catmullRomSpline(v, controlPoints[k - 1].x,
																	controlPoints[k    ].x,
																	controlPoints[k + 1].x,
																	controlPoints[k + 2].x), 0.f,
												catmullRomSpline(v, controlPoints[k - 1].y,
																	controlPoints[k    ].y,
																	controlPoints[k + 1].y,
																	controlPoints[k + 2].y));
					glm::vec3 global = rotation * (scalingFactor * local) + position;
					points.emplace_back(global);
				}
			points.emplace_back(points[0]);
			if (first) first = false;
			else for (int k = 0; k < numControlPoints * numStep; k++) {
					glBegin(GL_POLYGON);
					if (isPokeBall) {
						if (i <= 8) glColor3f(1.f, 1.f, 1.f);
						else if (i <= 16) {
							if (k >= (numControlPoints - 1) * numStep)
								glColor3f(0.f, 0.f, 0.f);
							else glColor3f(1.f, 1.f, 1.f);
						}
						else if (i <= 17) glColor3f(0.f, 0.f, 0.f);
						else if (i <= 26) {
							if (k >= (numControlPoints - 1) * numStep)
								glColor3f(0.f, 0.f, 0.f);
							else glColor3f(1.f, 0.f, 0.f);
						}
						else if (i <= 34) glColor3f(1.f, 0.f, 0.f);
						else if (i == 40) glColor3f(0.f, 0.f, 0.f);
						else glColor3f(1.f, 1.f, 1.f);
					}
					else glColor3f(red / 255.f, green / 255.f, blue / 255.f);
					glVertex3f(points[k].x, points[k].y, points[k].z);
					glVertex3f(lastPoints[k].x, lastPoints[k].y, lastPoints[k].z);
					glVertex3f(lastPoints[k + 1].x, lastPoints[k + 1].y, lastPoints[k + 1].z);
					glVertex3f(points[k + 1].x, points[k + 1].y, points[k + 1].z);
					glEnd();
				}
			lastPoints = points;
			switch (rgbState) {
				case 0:
					red--;
					green++;
					if (!red) rgbState = 1;
				break;

				case 1:
					green--;
					blue++;
					if (!green) rgbState = 2;
				break;

				default: // case 2
					blue--;
					red++;
					if (!blue) rgbState = 0;
				break;
			}
		}
}

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
				   1000.0 /* far clipping plane */);
	
	gluLookAt(dist * axisZ.x, dist * axisZ.y, dist * axisZ.z, 0.0, 0.0, 0.0,
			  axisY.x, axisY.y, axisY.z);
	glMatrixMode(GL_MODELVIEW);
}

void display()
{
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawSomething();
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
			float theta = speed * sqrt(d3);
			
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

void parse(char *filename)
{
	std::string input, line, curveType;

	if (!strcmp(filename, "poke_ball.txt")) isPokeBall = true;

	std::ifstream fin(filename);
	while(std::getline(fin, line)) {
		input += line.substr(0, line.find('#'));
		input.push_back(' ');
	}

	std::stringstream sin(input);
	sin >> curveType >> numCrossSection >> numControlPoints;
	isBSpline = (curveType == "BSPLINE");

	for (int i = 0; i < numCrossSection; i++) {
		std::vector<glm::vec2> controlPoints;
		for (int j = 0; j < numControlPoints; j++) {
			float x, z;
			sin >> x >> z;
			glm::vec2 controlPoint(x, z);
			controlPoints.emplace_back(controlPoint);
		}

		float x, y, z;
		float scalingFactor, angle;
		sin >> scalingFactor >> angle >> x >> y >> z;
		glm::quat rotation(1.f, 0.f, 0.f, 0.f);
		if (x || y || z)
			rotation = glm::rotate(rotation, angle, glm::vec3(x, y, z));
		sin >> x >> y >> z;
		glm::vec3 position(x, y, z);

		crossSections.emplace_back(controlPoints, scalingFactor, rotation, position);
	}

	crossSections.emplace(crossSections.begin(), crossSections[0]);
	crossSections.emplace_back(crossSections[numCrossSection]);
	// crossSections: 0 ~ numCrossSection + 1
}

int main(int argc, char** argv)
{
	if (argc <= 1) {
		std::cout << "expected: $ ./render <data filename>" << std::endl;
		return 0;
	}
	parse(argv[1]);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(width, height);
	glutCreateWindow("Swept Surface");

	reshape(width, height);
	if (isPokeBall) glClearColor(0.0, 0.3, 0.1, 1.0);
	else glClearColor(0.0, 0.0, 0.0, 1.0);

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
