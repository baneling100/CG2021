#ifndef LIGHT_H
#define LIGHT_H

#include <algorithm>

#include <GL/glut.h>

class Light
{
  public:
	Light(const GLenum _light, const GLfloat _ambient[4], const GLfloat _diffuse[4],
		  const GLfloat _specular[4], const GLfloat _position[4]);
	
	void set();

  public:
	GLenum light;
	GLfloat ambient[4], diffuse[4], specular[4], position[4];
};

#endif