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

Material emerald = Material((GLfloat[4]){0.0215f, 0.1745f, 0.0215f, 1.0f},
							(GLfloat[4]){0.07568f, 0.61424f, 0.07568f, 1.0f},
							(GLfloat[4]){0.633f, 0.727811f, 0.633f, 1.0f}, 0.6f);

Material jade = Material((GLfloat[4]){0.135f, 0.2225f, 0.1575f, 1.0f},
							(GLfloat[4]){0.54f, 0.89f, 0.63f, 1.0f},
							(GLfloat[4]){0.316228f, 0.316228f, 0.316228f, 1.0f}, 0.1f);

Material obsidian = Material((GLfloat[4]){0.05375f, 0.05f, 0.06625f, 1.0f},
							(GLfloat[4]){0.18275f, 0.17f, 0.22525f, 1.0f},
							(GLfloat[4]){0.296648f, 0.296648f, 0.296648f, 1.0f}, 0.088f);

Material pearl = Material((GLfloat[4]){0.25f, 0.20725f, 0.20725f, 1.0f},
							(GLfloat[4]){1.0f, 0.829f, 0.829f, 1.0f},
							(GLfloat[4]){0.633f, 0.727811f, 0.633f, 1.0f}, 0.6f);

Material ruby = Material((GLfloat[4]){0.1745f, 0.01175f, 0.01175f, 1.0f},
							(GLfloat[4]){0.61424f, 0.04136f, 0.04136f, 1.0f},
							(GLfloat[4]){0.727811f, 0.626959f, 0.626959f, 1.0f}, 0.6f);

Material turquoise = Material((GLfloat[4]){0.1f, 0.18725f, 0.1745f, 1.0f},
							(GLfloat[4]){0.396f, 0.74151f, 0.69102f, 1.0f},
							(GLfloat[4]){0.297254f, 0.30829f, 0.306678f, 1.0f}, 0.1f);

Material brass = Material((GLfloat[4]){0.329412f, 0.223529f, 0.027451f, 1.0f},
							(GLfloat[4]){0.780392f, 0.568627f, 0.113725f, 1.0f},
							(GLfloat[4]){0.992157f, 0.941176f, 0.807843f, 1.0f}, 0.21794872f);

Material bronze = Material((GLfloat[4]){0.2125f, 0.1275f, 0.054f, 1.0f},
							(GLfloat[4]){0.714f, 0.4284f, 0.18144f, 1.0f},
							(GLfloat[4]){0.393548f, 0.271906f, 0.166721f, 1.0f}, 0.2f);

Material chrome = Material((GLfloat[4]){0.25f, 0.25f, 0.25f, 1.0f},
							(GLfloat[4]){0.4f, 0.4f, 0.4f, 1.0f},
							(GLfloat[4]){0.774597f, 0.774597f, 0.774597f, 1.0f}, 0.6f);

Material copper = Material((GLfloat[4]){0.19125f, 0.0735f, 0.0225f, 1.0f},
							(GLfloat[4]){0.7038f, 0.27048f, 0.0828f, 1.0f},
							(GLfloat[4]){0.256777f, 0.137622f, 0.086014f, 1.0f}, 0.1f);

Material gold = Material((GLfloat[4]){0.24725f, 0.1995f, 0.0745f, 1.0f},
							(GLfloat[4]){0.75164f, 0.60648f, 0.22648f, 1.0f},
							(GLfloat[4]){0.628281f, 0.555802f, 0.366065f, 1.0f}, 0.4f);

Material silver = Material((GLfloat[4]){0.19225f, 0.19225f, 0.19225f, 1.0f},
							(GLfloat[4]){0.50754f, 0.50754f, 0.50754f, 1.0f},
							(GLfloat[4]){0.508273f, 0.508273f, 0.508273f, 1.0f}, 0.4f);

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

Material black_rubber = Material((GLfloat[4]){0.02f, 0.02f, 0.02f, 1.0f},
							(GLfloat[4]){0.01f, 0.01f, 0.01f, 1.0f},
							(GLfloat[4]){0.4f, 0.4f, 0.4f, 1.0f}, 0.078125f);

Material cyan_rubber = Material((GLfloat[4]){0.0f, 0.05f, 0.05f, 1.0f},
							(GLfloat[4]){0.4f, 0.5f, 0.5f, 1.0f},
							(GLfloat[4]){0.04f, 0.7f, 0.7f, 1.0f}, 0.078125f);

Material green_rubber = Material((GLfloat[4]){0.0f, 0.05f, 0.0f, 1.0f},
							(GLfloat[4]){0.4f, 0.5f, 0.4f, 1.0f},
							(GLfloat[4]){0.04f, 0.7f, 0.04f, 1.0f}, 0.078125f);

Material red_rubber = Material((GLfloat[4]){0.05f, 0.0f, 0.0f, 1.0f},
							(GLfloat[4]){0.5f, 0.4f, 0.4f, 1.0f},
							(GLfloat[4]){0.7f, 0.04f, 0.04f, 1.0f}, 0.078125f);

Material white_rubber = Material((GLfloat[4]){0.05f, 0.05f, 0.05f, 1.0f},
							(GLfloat[4]){0.5f, 0.5f, 0.5f, 1.0f},
							(GLfloat[4]){0.7f, 0.7f, 0.7f, 1.0f}, 0.078125f);

Material yellow_rubber = Material((GLfloat[4]){0.05f, 0.05f, 0.0f, 1.0f},
							(GLfloat[4]){0.5f, 0.5f, 0.4f, 1.0f},
							(GLfloat[4]){0.7f, 0.7f, 0.04f, 1.0f}, 0.078125f);
