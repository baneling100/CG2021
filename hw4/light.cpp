#include "light.h"

Light::Light(const GLenum _light, const GLfloat _ambient[4], const GLfloat _diffuse[4],
			 const GLfloat _specular[4], const GLfloat _position[4])
{
	light = _light;
	std::copy(_ambient, _ambient + 4, ambient);
	std::copy(_diffuse, _diffuse + 4, diffuse);
	std::copy(_specular, _specular + 4, specular);
	std::copy(_position, _position + 4, position);	
}

void Light::set()
{
	glEnable(light);
	glLightfv(light, GL_AMBIENT, ambient);
	glLightfv(light, GL_DIFFUSE, diffuse);
	glLightfv(light, GL_SPECULAR, specular);
	glLightfv(light, GL_POSITION, position);
}

Light light0 = Light(GL_LIGHT0, (GLfloat[4]){0.2f, 0.2f, 0.2f, 1.0f},
								(GLfloat[4]){0.7f, 0.7f, 0.7f, 1.0f},
								(GLfloat[4]){0.4f, 0.4f, 0.4f, 1.0f},
								(GLfloat[4]){0.0f, 0.0f, 10000.0f, 1.0f});
Light light1 = Light(GL_LIGHT1, (GLfloat[4]){0.2f, 0.2f, 0.2f, 1.0f},
								(GLfloat[4]){0.7f, 0.7f, 0.7f, 1.0f},
								(GLfloat[4]){0.4f, 0.4f, 0.4f, 1.0f},
								(GLfloat[4]){0.0f, 10000.0f, 0.0f, 1.0f});
Light light2 = Light(GL_LIGHT2, (GLfloat[4]){0.2f, 0.2f, 0.2f, 1.0f},
								(GLfloat[4]){0.7f, 0.7f, 0.7f, 1.0f},
								(GLfloat[4]){0.4f, 0.4f, 0.4f, 1.0f},
								(GLfloat[4]){0.0f, -10000.0f, 0.0f, 1.0f});
