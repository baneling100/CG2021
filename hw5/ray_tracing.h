#ifndef RAY_TRACING_H
#define RAY_TRACING_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "light.h"
#include "model.h"

glm::vec3 ray_trace(glm::vec3 &p0, glm::vec3 &u, Light *lights[], Model *models[], Images &images, int depth, bool is_air);

#endif
