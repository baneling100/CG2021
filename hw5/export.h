#ifndef EXPORT_H
#define EXPORT_H

#include <string>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <vector>
#include <mutex>

#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "light.h"
#include "model.h"
#include "ray_tracing.h"

void export_opengl_image(std::string &name, int width, int height);

void export_raytraced_image(std::string &name, int width, int height, float fovy, glm::vec3 &camera, glm::vec3 &origin, glm::vec3 &axisY, Light *lights[], Model *models[], Images &images, int num_threads);
void export_soft_shadows_image(std::string &name, int width, int height, float fovy, glm::vec3 &camera, glm::vec3 &origin, glm::vec3 &axisY, Light *lights[], Model *models[], Images &images, int num_threads);
void export_depth_of_field_image(std::string &name, int width, int height, float fovy, glm::vec3 &camera, glm::vec3 &origin, glm::vec3 &axisY, Light *lights[], Model *models[], Images &images, int num_threads);
void export_motion_blur_image(std::string &name, int width, int height, float fovy, glm::vec3 &camera, glm::vec3 &origin, glm::vec3 &axisY, Light *lights[], Model *models[], Images &images, int num_threads);
void export_bump_mapping_image(std::string &name, int width, int height, float fovy, glm::vec3 &camera, glm::vec3 &origin, glm::vec3 &axisY, Light *lights[], Model *models[], Images &images, int num_threads);

#endif
