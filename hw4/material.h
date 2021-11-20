#ifndef MATERIAL_H
#define MATERIAL_H

#include <algorithm>

#include <GL/glut.h>

class Material
{
  public:
	Material(const GLfloat _ambient[4], const GLfloat _diffuse[4],
			 const GLfloat _specular[4], const GLfloat _shininess);

	void set();

  private:
	GLfloat ambient[4], diffuse[4], specular[4], shininess;
};

extern Material emerald;
extern Material jade;
extern Material obsidian;
extern Material pearl;
extern Material ruby;
extern Material turquoise;
extern Material brass;
extern Material bronze;
extern Material chrome;
extern Material copper;
extern Material gold;
extern Material silver;
extern Material black_plastic;
extern Material cyan_plastic;
extern Material green_plastic;
extern Material red_plastic;
extern Material white_plastic;
extern Material white_plastic_trans;
extern Material yellow_plastic;
extern Material purple_plastic;
extern Material pink_plastic;
extern Material black_rubber;
extern Material cyan_rubber;
extern Material green_rubber;
extern Material red_rubber;
extern Material white_rubber;
extern Material yellow_rubber;

#endif
