#ifndef MATERIAL_H
#define MATERIAL_H

#include <algorithm>

#include <GL/glut.h>

class Material
{
  public:
	Material(const GLfloat _ambient[4], const GLfloat _diffuse[4],
			 const GLfloat _specular[4], const GLfloat _shininess,
			 const GLfloat _reflect);

	void set();
	
	GLfloat ambient[4], diffuse[4], specular[4], shininess, reflect;
};

extern Material default_material;
extern Material black_plastic;
extern Material cyan_plastic;
extern Material green_plastic;
extern Material red_plastic;
extern Material white_plastic;
extern Material glass;
extern Material mirror;
extern Material yellow_plastic;
extern Material purple_plastic;
extern Material pink_plastic;

#endif
