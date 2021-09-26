import binascii
from OpenGL.GLUT import *
from OpenGL.GL import *
from OpenGL.GLU import *

width = 400
height = 400
leftButton=False
mousePosX,mousePosY = 0,0
timeStep = 30
eye = [ 0.0, 0.0, 100.0 ];
ori = [ 0.0, 0.0, 0.0 ];
rot = [ 0.0, 0.0, 0.0 ];

def loadGlobalCoord():
	glLoadIdentity()
	gluLookAt(eye[0], eye[1], eye[2], ori[0], ori[1], ori[2], 0, 1, 0)

def glutMotion(x,y):
	global leftButton,mousePosX,mousePosY
	
	if(leftButton):
		dx = x - mousePosX;
		dy = y - mousePosY;

		mousePosX = x;
		mousePosY = y;

		ori[0] -= dx*0.04;
		ori[1] += dy*0.04;

	loadGlobalCoord();

def glutMouse(button, state, x, y):
	global leftButton,mousePosX,mousePosY

	if button == GLUT_LEFT_BUTTON:
		if state == GLUT_DOWN:
			leftButton = True
			mousePosX,mousePosY = x,y
		if state == GLUT_UP:
			leftButton = False

	if button == GLUT_RIGHT_BUTTON:
		return

def resize(w, h):
	width = w
	height = h
	glViewport(0, 0, w, h)
	glMatrixMode(GL_PROJECTION)
	glLoadIdentity()
	gluPerspective(45.0, w / h, 0.1, 500.0)
	glMatrixMode(GL_MODELVIEW)
	glLoadIdentity()

def glutKeyboard(key, x, y):
	key = int(binascii.hexlify(key).decode(),16)

	if key == 27:
		exit()

def Timer(unused):
	glutPostRedisplay()
	glutTimerFunc(timeStep, Timer, 0)


def display():
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
	glEnable(GL_DEPTH_TEST)
	glClearColor(0.1, 0.1, 0.1, 1.0)
	loadGlobalCoord()

	glRotatef( 90.0, 0.0, 0.0, 1.0)
	glPushMatrix()


	# triangle with multiple colors
	glBegin(GL_TRIANGLES)
	glColor3f(1.0, 0.0, 0.0)
	glVertex3f(-18.0, -12.0, 0.0)
	glColor3f(0.0, 1.0, 0.0)
	glVertex3f(18.0, -12.0, 0.0)
	glColor3f(0.0, 0.0, 1.0)
	glVertex3f(0.0, 18.0, 0.0)
	glEnd()

	# WHITE
	glBegin(GL_POLYGON)
	glColor3f(   1.0,  1.0, 1.0 )
	glVertex3f(  5, -5, 5 )
	glVertex3f(  5,  5, 5 );
	glVertex3f( -5,  5, 5 )
	glVertex3f( -5, -5, 5 )
	glEnd()

	# RED
	glBegin(GL_POLYGON)
	glColor3f(   1.0,  0.0,  0.0 )
	glVertex3f( -5, -5,  5 )
	glVertex3f( -5,  5,  5 )
	glVertex3f( -5,  5, -5 )
	glVertex3f( -5, -5, -5 )
	glEnd();

	# GREEN
	glBegin(GL_POLYGON)
	glColor3f(  0.0,  1.0,  0.0 )
	glVertex3f( 5, -5, -5 )
	glVertex3f( 5,  5, -5 )
	glVertex3f( 5,  5,  5 )
	glVertex3f( 5, -5,  5 )
	glEnd();


	# BLUE
	glBegin(GL_POLYGON);
	glColor3f(   0.0,  0.0,  1.0 )
	glVertex3f(  5,  5,  5 )
	glVertex3f(  5,  5, -5 )
	glVertex3f( -5,  5, -5 )
	glVertex3f( -5,  5,  5 )
	glEnd()


	# YELLOW
	glBegin(GL_POLYGON)
	glColor3f(   1.0,  1.0,  0.0 )
	glVertex3f(  5, -5, -5 )
	glVertex3f(  5, -5,  5 )
	glVertex3f( -5, -5,  5 )
	glVertex3f( -5, -5, -5 )
	glEnd()

	glColor3f(1.0, 1.0, 1.0)
	glBegin(GL_LINE_STRIP)
	glVertex3f(10.0, 10.0, 0.0)
	glVertex3f(-10.0, 10.0, 0.0)
	glVertex3f(-10.0, -10.0, 0.0)
	glVertex3f(10.0, -10.0, 0.0)
	glVertex3f(10.0, 10.0, 0.0)
	glEnd()

	glPushMatrix()
		
	glRotatef(45.0, 0.0, 0.0, 1.0)
	glTranslatef(10.0, 10.0, 0.0)
	glColor3f(1.0, 0.0, 0.0)
	glBegin(GL_LINE_STRIP)
	glVertex3f(10.0, 10.0, 0.0)
	glVertex3f(-10.0, 10.0, 0.0)
	glVertex3f(-10.0, -10.0, 0.0)
	glVertex3f(10.0, -10.0, 0.0)
	glVertex3f(10.0, 10.0, 0.0)
	glEnd()

	glPushMatrix()
		
	glRotatef(45.0, 0.0, 0.0, 1.0)
	glTranslatef(10.0, 10.0, 0.0)
	glColor3f(0.0, 0.0, 1.0)
	glBegin(GL_LINE_STRIP)
	glVertex3f(10.0, 10.0, 0.0)
	glVertex3f(-10.0, 10.0, 0.0)
	glVertex3f(-10.0, -10.0, 0.0)
	glVertex3f(10.0, -10.0, 0.0)
	glVertex3f(10.0, 10.0, 0.0)
	glEnd()
	
	glPopMatrix()
	

	glPopMatrix()

	glPopMatrix()


	glutSwapBuffers()
	

if __name__ == "__main__":
	glutInit()
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
	glutInitWindowSize(width, height)
	glutInitWindowPosition( 50, 0 )
	glutCreateWindow("Example")

	glutReshapeFunc(resize)
	glutDisplayFunc(display)
	glutTimerFunc(timeStep, Timer, 0)
	glutKeyboardFunc(glutKeyboard)
	glutMouseFunc(glutMouse)
	glutMotionFunc(glutMotion)

	glutMainLoop()

