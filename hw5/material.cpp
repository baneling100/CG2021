#include "material.h"

Material::Material(const GLfloat _ambient[4], const GLfloat _diffuse[4],
				   const GLfloat _specular[4], const GLfloat _shininess)
{
	std::copy(_ambient, _ambient + 4, ambient);
	std::copy(_diffuse, _diffuse + 4, diffuse);
	std::copy(_specular, _specular + 4, specular);
	shininess = _shininess;
}

void Material::set()
{
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &shininess);
}

Material texture_material = Material((GLfloat[4]){0.2f, 0.2f, 0.2f, 1.0f},
							(GLfloat[4]){0.8f, 0.8f, 0.8f, 0.8f},
							(GLfloat[4]){1.0f, 1.0f, 1.0f, 1.0f}, 0.0f);

Material black_plastic = Material((GLfloat[4]){0.0f, 0.0f, 0.0f, 1.0f},
							(GLfloat[4]){0.01f, 0.01f, 0.01f, 1.0f},
							(GLfloat[4]){0.50f, 0.50f, 0.50f, 1.0f}, 0.25f);

Material cyan_plastic = Material((GLfloat[4]){0.0f, 0.1f, 0.06f, 1.0f},
							(GLfloat[4]){0.0f, 0.50980392f, 0.50980392f, 1.0f},
							(GLfloat[4]){0.50196078f, 0.50196078f, 0.50196078f, 1.0f}, 0.25f);

Material green_plastic = Material((GLfloat[4]){0.0f, 0.0f, 0.0f, 1.0f},
							(GLfloat[4]){0.1f, 0.35f, 0.1f, 1.0f},
							(GLfloat[4]){0.45f, 0.55f, 0.45f, 1.0f}, 0.25f);

Material red_plastic = Material((GLfloat[4]){0.0f, 0.0f, 0.0f, 1.0f},
							(GLfloat[4]){0.5f, 0.0f, 0.0f, 1.0f},
							(GLfloat[4]){0.7f, 0.6f, 0.6f, 1.0f}, 0.25f);

Material white_plastic = Material((GLfloat[4]){0.0f, 0.0f, 0.0f, 1.0f},
							(GLfloat[4]){0.55f, 0.55f, 0.55f, 1.0f},
							(GLfloat[4]){0.70f, 0.70f, 0.70f, 1.0f}, 0.25f);

Material white_plastic_trans = Material((GLfloat[4]){0.0f, 0.0f, 0.0f, 1.0f},
							(GLfloat[4]){0.55f, 0.55f, 0.55f, 0.3f},
							(GLfloat[4]){0.70f, 0.70f, 0.70f, 1.0f}, 0.25f);

Material yellow_plastic = Material((GLfloat[4]){0.0f, 0.0f, 0.0f, 1.0f},
							(GLfloat[4]){0.5f, 0.5f, 0.0f, 1.0f},
							(GLfloat[4]){0.60f, 0.60f, 0.50f, 1.0f}, 0.25f);

Material purple_plastic = Material((GLfloat[4]){0.0f, 0.0f, 0.0f, 1.0f},
							(GLfloat[4]){0.15f, 0.05f, 0.6f, 1.0f},
							(GLfloat[4]){0.55f, 0.50f, 0.60f, 1.0f}, 0.25f);

Material pink_plastic = Material((GLfloat[4]){0.0f, 0.0f, 0.0f, 1.0f},
							(GLfloat[4]){0.5f, 0.0f, 0.5f, 1.0f},
							(GLfloat[4]){0.60f, 0.50f, 0.60f, 1.0f}, 0.25f);
							