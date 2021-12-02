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
