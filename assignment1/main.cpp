#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

/* set global variables */
int width, height;

bool leftButton = false;
GLfloat mousePosX, mousePosY;
unsigned timeStep = 30;

/* vectors that makes the rotation and translation of the cube */
float eye[3] = { 100.0f, 50.0f, 100.0f };
float ori[3] = { 0.0f, 0.0f, 0.0f };
float rot[3] = { 0.0f, 0.0f, 0.0f };

/**
 * @brief joint[0]: thumb, joint[1]: forefinger, joint[2]: middle finger,
 *        joint[3]: ring finger, joint[4]: little finger
 *        joint[i][j] is relative to joint[i][j - 1]
 */
struct Data {
	GLfloat x;
	GLfloat y;
	GLfloat pivot_x;
	GLfloat pivot_y;
	GLfloat fold_angle;
} joint[5][5] = {{{-91, -29, 0, 46, 80.f}, {-106, 127, 61, 26, 45.f}, {-44, 95, 46, 20, 45.f}, {-34, 67, 42, 21, 0.f}, {0, 0, 1.f, 0.f, 0.f}},
				 {{-39, -2, 54, 13, 0.f}, {-44, 222, 53, 7, 90.f}, {-9, 141, 41, 3, 110.f}, {-2, 82, 42, -1, 90.f}, {0, 59, 48, -2, 0.f}},
				 {{0, 0, 46, 2, 0.f}, {-4, 222, 32, 0, 90.f}, {6, 156, 34, -1, 110.f}, {4, 98, 35, -2, 90.f}, {4, 58, 43, -5, 0.f}},
				 {{38, -4, 32, -9, 0.f}, {28, 199, 49, -2, 90.f}, {7, 145, 35, -4, 110.f}, {11, 97, 36, -3, 90.f}, {5, 60, 34, -5, 0.f}},
				 {{65, -20, 34, 24, 0.f}, {65, 183, 46, -9, 90.f}, {17, 113, 35, -9, 110.f}, {11, 63, 32, -8, 90.f}, {6, 52, 38, -5, 0.f}}};
GLfloat scale = 0.05, dist = 40.f, high = 10.f;
GLfloat p1 = 500.f, t1 = 2.5 * p1, p3 = 1000.f, t3 = p3;
GLfloat p = 30000.f, t = 0.f, depth = 1.f;

void init()
{
	for (int i = 0; i < 5; i++)
		for (int j = 0; j < 5; j++) {
			joint[i][j].x *= scale;
			joint[i][j].y *= scale;
			double size = sqrt(joint[i][j].pivot_x * joint[i][j].pivot_x + joint[i][j].pivot_y * joint[i][j].pivot_y);
			joint[i][j].pivot_x /= size;
			joint[i][j].pivot_y /= size;
		}
}

void loadGlobalCoord()
{
	glLoadIdentity();
	gluLookAt(eye[0] * cos(2.0 * M_PI * t / p), eye[1], eye[2] * sin(2.0 * M_PI * t / p), ori[0], ori[1], ori[2], 0, 1, 0);
}

//------------------------------------------------------------------------
// Moves the screen based on mouse pressed button
//------------------------------------------------------------------------

void glutMotion(int x, int y)
{
	if ( leftButton ) {
		float dx = x - mousePosX;
		float dy = y - mousePosY;

		mousePosX = x;
		mousePosY = y;

		//ori[0] -= dx*0.04;
		//ori[1] += dy*0.04;
		ori[0] -= dx * 0.1;
		ori[1] += dy * 0.1;

		loadGlobalCoord();
	}
	return;
}

//------------------------------------------------------------------------
// Function that handles mouse input
//------------------------------------------------------------------------
void glutMouse(int button, int state, int x, int y)
{
	switch ( button )
	{
		case GLUT_LEFT_BUTTON:
			if ( state == GLUT_DOWN )
			{
				mousePosX = x;
				mousePosY = y;
				leftButton = true;
			}
			else if ( state == GLUT_UP )
			{
				leftButton = false;
			}
			break;
		case GLUT_RIGHT_BUTTON:break;
		case 3:break;
		default:break;
	}
	return;
}

void draw_hexahedron(glm::vec4 u1, glm::vec4 u2, glm::vec4 u3, glm::vec4 u4,
					 glm::vec4 v1, glm::vec4 v2, glm::vec4 v3, glm::vec4 v4)
{
	// u1 u2 u3 u4
	glBegin(GL_POLYGON);
	glColor3f(224.f / 255.f, 225.f / 255.f, 215.f / 255.f);
	glVertex3f(u1.x, u1.y, u1.z);
	glVertex3f(u2.x, u2.y, u2.z);
	glVertex3f(u3.x, u3.y, u3.z);
	glVertex3f(u4.x, u4.y, u4.z);
	glEnd();
	
	// v1 v2 v3 v4
	glBegin(GL_POLYGON);
	glColor3f(224.f / 255.f, 225.f / 255.f, 215.f / 255.f);
	glVertex3f(v1.x, v1.y, v1.z);
	glVertex3f(v2.x, v2.y, v2.z);
	glVertex3f(v3.x, v3.y, v3.z);
	glVertex3f(v4.x, v4.y, v4.z);
	glEnd();

	// u1 u2 v2 v1
	glBegin(GL_POLYGON);
	glColor3f(224.f / 255.f, 225.f / 255.f, 215.f / 255.f);
	glVertex3f(u1.x, u1.y, u1.z);
	glVertex3f(u2.x, u2.y, u2.z);
	glVertex3f(v2.x, v2.y, v2.z);
	glVertex3f(v1.x, v1.y, v1.z);
	glEnd();

	// u2 u3 v3 v2
	glBegin(GL_POLYGON);
	glColor3f(224.f / 255.f, 225.f / 255.f, 215.f / 255.f);
	glVertex3f(u2.x, u2.y, u2.z);
	glVertex3f(u3.x, u3.y, u3.z);
	glVertex3f(v3.x, v3.y, v3.z);
	glVertex3f(v2.x, v2.y, v2.z);
	glEnd();

	// u3 u4 v4 v3
	glBegin(GL_POLYGON);
	glColor3f(224.f / 255.f, 225.f / 255.f, 215.f / 255.f);
	glVertex3f(u3.x, u3.y, u3.z);
	glVertex3f(u4.x, u4.y, u4.z);
	glVertex3f(v4.x, v4.y, v4.z);
	glVertex3f(v3.x, v3.y, v3.z);
	glEnd();

	// u4 u1 v1 v4
	glBegin(GL_POLYGON);
	glColor3f(224.f / 255.f, 225.f / 255.f, 215.f / 255.f);
	glVertex3f(u4.x, u4.y, u4.z);
	glVertex3f(u1.x, u1.y, u1.z);
	glVertex3f(v1.x, v1.y, v1.z);
	glVertex3f(v4.x, v4.y, v4.z);
	glEnd();
}

void draw_line(glm::vec4 u1, glm::vec4 u2)
{
	glBegin(GL_LINES);
	glColor3f(0.f, 0.f, 0.f);
	glVertex3f(u1.x, u1.y, u1.z);
	glVertex3f(u2.x, u2.y, u2.z);
	glEnd();
}

void draw_hand(bool enable[])
{
	for (int i = 0; i < 5; i++) {
		for (int j = 0; i ? j < 4 : j < 3; j++) {
			glPushMatrix();
				// draw skeleton
				glTranslatef(joint[i][j].x, joint[i][j].y, 0.f);
				GLfloat angle = (enable[i] && t1 < 0.25 * p1 ? 4.0 * t1 / p1:
								1.f) * joint[i][j].fold_angle;
				glRotatef(angle, joint[i][j].pivot_x, joint[i][j].pivot_y, 0.f);
				glBegin(GL_LINES);
				glColor3f(1.f, 1.f, 1.f);
				glVertex3f(0.f, 0.f, 0.f);
				glVertex3f(joint[i][j + 1].x, joint[i][j + 1].y, 0.f);
				glEnd();

				// draw finger flesh
				glm::mat4 mat1 = glm::rotate(glm::mat4(1.f),
											 glm::radians(-angle / 2.f),
											 glm::vec3(joint[i][j].pivot_x,
											 		   joint[i][j].pivot_y,
													   0.f));
				glm::vec4 u1 = mat1 * glm::vec4(-depth * joint[i][j].pivot_x,
												-depth * joint[i][j].pivot_y,
												depth,
												1.f);
				glm::vec4 u2 = mat1 * glm::vec4(depth * joint[i][j].pivot_x,
												depth * joint[i][j].pivot_y,
												depth,
												1.f);
				glm::vec4 u3 = mat1 * glm::vec4(depth * joint[i][j].pivot_x,
												depth * joint[i][j].pivot_y,
												-depth,
												1.f);
				glm::vec4 u4 = mat1 * glm::vec4(-depth * joint[i][j].pivot_x,
												-depth * joint[i][j].pivot_y,
												-depth,
												1.f);
				glm::mat4 mat2 = glm::translate(glm::mat4(1.f),
												glm::vec3(joint[i][j + 1].x,
														  joint[i][j + 1].y,
														  0.f));
				GLfloat next_angle = (enable[i] && t1 < 0.25 * p1 ? 4.0 * t1
									 / p1 : 1.f) * joint[i][j + 1].fold_angle;
				mat2 = glm::rotate(mat2, glm::radians(next_angle / 2.f),
								   glm::vec3(joint[i][j + 1].pivot_x,
								   			 joint[i][j + 1].pivot_y,
											 0.f));
				glm::vec4 v1 = mat2 * glm::vec4(-depth * joint[i][j + 1].pivot_x,
												-depth * joint[i][j + 1].pivot_y,
												depth,
												1.f);
				glm::vec4 v2 = mat2 * glm::vec4(depth * joint[i][j + 1].pivot_x,
												depth * joint[i][j + 1].pivot_y,
												depth,
												1.f);
				glm::vec4 v3 = mat2 * glm::vec4(depth * joint[i][j + 1].pivot_x,
												depth * joint[i][j + 1].pivot_y,
												-depth,
												1.f);
				glm::vec4 v4 = mat2 * glm::vec4(-depth * joint[i][j + 1].pivot_x,
												-depth * joint[i][j + 1].pivot_y,
												-depth,
												1.f);
				draw_hexahedron(u1, u2, u3, u4, v1, v2, v3, v4);
				draw_line(v1, v2);
				draw_line(v2, v3);
				draw_line(v3, v4);
				draw_line(v4, v1);
				if (j > 0) {
					draw_line(u1, v1);
					draw_line(u2, v2);
					draw_line(u3, v3);
					draw_line(u4, v4);
				}
				else if (i == 0) {
					draw_line(u1, v1);
					draw_line(u4, v4);
				}
				else if (i == 4) {
					draw_line(u2, v2);
					draw_line(u3, v3);
				}
		}
		for (int j = 0; i ? j < 4 : j < 3; j++)
			glPopMatrix();
	}

	// draw palm flesh
	for (int i = 0; i < 4; i++) { // i ~ i + 1
		GLfloat angle1 = (enable[i] && t1 < 0.25 * p1 ? 4.0 * t1 / p1: 1.f) *
						joint[i][0].fold_angle;
		glm::mat4 mat1 = glm::translate(glm::mat4(1.f),
										glm::vec3(joint[i][0].x,
												  joint[i][0].y,
												  0.f));
		glm::mat4 mat2 = glm::rotate(mat1, glm::radians(angle1),
									 glm::vec3(joint[i][0].pivot_x,
									 		   joint[i][0].pivot_y,
											   0.f));
		mat1 = glm::rotate(mat1, glm::radians(angle1 / 2.f),
						   glm::vec3(joint[i][0].pivot_x,
									 joint[i][0].pivot_y,
									 0.f));
		mat2 = glm::translate(mat2, glm::vec3(joint[i][1].x, joint[i][1].y, 0.f));
		GLfloat angle2 = (enable[i] && t1 < 0.25 * p1 ? 4.0 * t1 / p1: 1.f) *
						 joint[i][1].fold_angle;
		mat2 = glm::rotate(mat2, glm::radians(angle2 / 2.f),
						   glm::vec3(joint[i][1].pivot_x,
						   			 joint[i][1].pivot_y,
									 0.f));
		glm::vec4 u1 = mat1 * glm::vec4(depth * joint[i][0].pivot_x,
										depth * joint[i][0].pivot_y,
										depth,
										1.f);
		glm::vec4 u4 = mat1 * glm::vec4(depth * joint[i][0].pivot_x,
										depth * joint[i][0].pivot_y,
										-depth,
										1.f);
		glm::vec4 v1 = mat2 * glm::vec4(depth * joint[i][1].pivot_x,
										depth * joint[i][1].pivot_y,
										depth,
										1.f);
		glm::vec4 v4 = mat2 * glm::vec4(depth * joint[i][1].pivot_x,
										depth * joint[i][1].pivot_y,
										-depth,
										1.f);
		angle1 = (enable[i + 1] && t1 < 0.25 * p1 ? 4.0 * t1 / p1: 1.f) *
						joint[i + 1][0].fold_angle;
		mat1 = glm::translate(glm::mat4(1.f),
							  glm::vec3(joint[i + 1][0].x,
							  joint[i + 1][0].y,
							  0.f));
		mat2 = glm::rotate(mat1, glm::radians(angle1),
						   glm::vec3(joint[i + 1][0].pivot_x,
						   joint[i + 1][0].pivot_y,
						   0.f));
		mat1 = glm::rotate(mat1, glm::radians(angle1 / 2.f),
						   glm::vec3(joint[i + 1][0].pivot_x,
									 joint[i + 1][0].pivot_y,
									 0.f));
		mat2 = glm::translate(mat2, glm::vec3(joint[i + 1][1].x,
											  joint[i + 1][1].y,
											  0.f));
		angle2 = (enable[i + 1] && t1 < 0.25 * p1 ? 4.0 * t1 / p1: 1.f) *
						 joint[i + 1][1].fold_angle;
		mat2 = glm::rotate(mat2, glm::radians(angle2 / 2.f),
						   glm::vec3(joint[i + 1][1].pivot_x,
						   			 joint[i + 1][1].pivot_y,
									 0.f));
		glm::vec4 u2 = mat1 * glm::vec4(-depth * joint[i + 1][0].pivot_x,
										-depth * joint[i + 1][0].pivot_y,
										depth,
										1.f);
		glm::vec4 u3 = mat1 * glm::vec4(-depth * joint[i + 1][0].pivot_x,
										-depth * joint[i + 1][0].pivot_y,
										-depth,
										1.f);
		glm::vec4 v2 = mat2 * glm::vec4(-depth * joint[i + 1][1].pivot_x,
										-depth * joint[i + 1][1].pivot_y,
										depth,
										1.f);
		glm::vec4 v3 = mat2 * glm::vec4(-depth * joint[i + 1][1].pivot_x,
										-depth * joint[i + 1][1].pivot_y,
										-depth,
										1.f);
		draw_hexahedron(u1, u2, u3, u4, v1, v2, v3, v4);
		draw_line(v1, v2);
		draw_line(v2, v3);
		draw_line(v3, v4);
		draw_line(v4, v1);
		if (i == 0) {
			draw_line(u1, v1);
			draw_line(u2, v2);
			draw_line(u3, v3);
			draw_line(u4, v4);
		}
	}
}

void draw_three_hands()
{
	// rock
	glPushMatrix();
	glRotatef(-90.f, 0.f, 0.f, 1.f);
	glRotatef(180.f, 1.f, 0.f, 0.f);
	glTranslatef(0.f, -dist, 0.f);
	glTranslatef(-high * sin(2.0 * M_PI * t1 / p1), 0.f, 0.f);
	bool rock[5] = {false, false, false, false, false};
	draw_hand(rock);
	glPopMatrix();

	// paper
	glPushMatrix();
	glRotatef(-90.f, 0.f, 0.f, 1.f);
	glRotatef(-60.f, 1.f, 0.f, 0.f);
	glTranslatef(0.f, -dist, 0.f);
	glTranslatef(-high * sin(2.0 * M_PI * t1 / p1), 0.f, 0.f);
	bool paper[5] = {true, true, true, true, true};
	draw_hand(paper);
	glPopMatrix();

	// scissors
	glPushMatrix();
	glRotatef(-90.f, 0.f, 0.f, 1.f);
	glRotatef(60.f, 1.f, 0.f, 0.f);
	glTranslatef(0.f, -dist, 0.f);
	glTranslatef(-high * sin(2.0 * M_PI * t1 / p1), 0.f, 0.f);
	bool scissors[5] = {false, true, true, false, false};
	draw_hand(scissors);
	glPopMatrix();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	loadGlobalCoord();
	draw_three_hands();
	glutSwapBuffers();
}

void resize(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)w / (GLfloat)h, .1f, 500.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 27:
		exit(0);
		break;
	default:
		break;
	}
}



void Timer(int unused)
{
	/* call the display callback and forces the current window to be displayed */
	glutPostRedisplay();
	t += timeStep;
	if (t1 > 0) {
		t1 -= timeStep;
		if (t1 < 0)
			t1 = 0;
	}
	else if (t3 > 0) {
		t3 -= timeStep;
		if (t3 < 0) {
			t1 = 2.5 * p1;
			t3 = p3;
		}
	}
	glutTimerFunc(timeStep, Timer, 0);
}

int main(int argc, char **argv)
{
	init();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 800);
	glutInitWindowPosition( 50, 0 );
	glutCreateWindow("Rock-Paper-Scissors");

	glutReshapeFunc(resize);
	glutDisplayFunc(display);
	glutTimerFunc(timeStep, Timer, 0);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(glutMouse);
	glutMotionFunc(glutMotion);

	glutMainLoop();

	return 0;
}
