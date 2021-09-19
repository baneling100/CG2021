#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#include <iostream>
#include <cmath>
#include <algorithm>

/* set global variables */
int width, height;

bool leftButton = false;
GLfloat mousePosX, mousePosY;
unsigned timeStep = 30;

/* vectors that makes the rotation and translation of the cube */
float eye[3] = { 0.0f, 0.0f, 100.0f };
float ori[3] = { 0.0f, 0.0f, 0.0f };
float rot[3] = { 0.0f, 0.0f, 0.0f };

void loadGlobalCoord()
{
	glLoadIdentity();
	gluLookAt(eye[0], eye[1], eye[2], ori[0], ori[1], ori[2], 0, 1, 0);
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

		ori[0] -= dx*0.04;
		ori[1] += dy*0.04;

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

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	loadGlobalCoord();
	
		
	glRotatef((float) 90.0f, 0.f, 0.f, 1.f);
	glPushMatrix();


	// triangle with multiple colors
	glBegin(GL_TRIANGLES);
	glColor3f(1.f, 0.f, 0.f);
	glVertex3f(12.f, -12.f, 0.f);
    	glColor3f(0.f, 1.f, 0.f);
    	glVertex3f(48.f, -12.f, 0.f);
    	glColor3f(0.f, 0.f, 1.f);
    	glVertex3f(30.f, 18.f, 0.f);
    	glEnd();
	
	
	// WHITE
	glBegin(GL_POLYGON);
	glColor3f(   1.0,  1.0, 1.0 );
	glVertex3f(  5, -5, 5 );
	glVertex3f(  5,  5, 5 );
	glVertex3f( -5,  5, 5 );
	glVertex3f( -5, -5, 5 );
	glEnd();
	
	// RED
	glBegin(GL_POLYGON);
	glColor3f(   1.0,  0.0,  0.0 );
	glVertex3f( -5, -5,  5 );
	glVertex3f( -5,  5,  5 );
	glVertex3f( -5,  5, -5 );
	glVertex3f( -5, -5, -5 );
	glEnd();

	// GREEN
	glBegin(GL_POLYGON);
	glColor3f(  0.0,  1.0,  0.0 );
	glVertex3f( 5, -5, -5 );
	glVertex3f( 5,  5, -5 );
	glVertex3f( 5,  5,  5 );
	glVertex3f( 5, -5,  5 );
	glEnd();


	// BLUE
	glBegin(GL_POLYGON);
	glColor3f(   0.0,  0.0,  1.0 );
	glVertex3f(  5,  5,  5 );
	glVertex3f(  5,  5, -5 );
	glVertex3f( -5,  5, -5 );
	glVertex3f( -5,  5,  5 );
	glEnd();

	
	// YELLOW
	glBegin(GL_POLYGON);
	glColor3f(   1.0,  1.0,  0.0 );
	glVertex3f(  5, -5, -5 );
	glVertex3f(  5, -5,  5 );
	glVertex3f( -5, -5,  5 );
	glVertex3f( -5, -5, -5 );
	glEnd();
	
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_LINE_STRIP);
	glVertex3f(10.0, 10.0, 0.0);
	glVertex3f(-10.0, 10.0, 0.0);
	glVertex3f(-10.0, -10.0, 0.0);
	glVertex3f(10.0, -10.0, 0.0);
	glVertex3f(10.0, 10.0, 0.0);
	glEnd();

	glPushMatrix();
	{	
		glRotatef(45.0, 0.0, 0.0, 1.0);	
		glTranslatef(10.0, 10.0, 0.0);
		glColor3f(1.0, 0.0, 0.0);
		glBegin(GL_LINE_STRIP);
		glVertex3f(10.0, 10.0, 0.0);
		glVertex3f(-10.0, 10.0, 0.0);
		glVertex3f(-10.0, -10.0, 0.0);
		glVertex3f(10.0, -10.0, 0.0);
		glVertex3f(10.0, 10.0, 0.0);
		glEnd();

		glPushMatrix();
		{	
			glRotatef(45.0, 0.0, 0.0, 1.0);	
			glTranslatef(10.0, 10.0, 0.0);
			glColor3f(0.0, 0.0, 1.0);
			glBegin(GL_LINE_STRIP);
			glVertex3f(10.0, 10.0, 0.0);
			glVertex3f(-10.0, 10.0, 0.0);
			glVertex3f(-10.0, -10.0, 0.0);
			glVertex3f(10.0, -10.0, 0.0);
			glVertex3f(10.0, 10.0, 0.0);
			glEnd();
		}
		glPopMatrix();
	}
	glPopMatrix();

	glutSwapBuffers();
}

void resize(int w, int h) {
	width = w;
	height = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)w / (GLfloat)h, .1f, 500.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y) {
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
	glutTimerFunc(timeStep, Timer, 0);
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(400, 400);
	glutInitWindowPosition( 50, 0 );
	glutCreateWindow("Example");

	glutReshapeFunc(resize);
	glutDisplayFunc(display);
	glutTimerFunc(timeStep, Timer, 0);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(glutMouse);
	glutMotionFunc(glutMotion);

	glutMainLoop();

	return 0;
}
