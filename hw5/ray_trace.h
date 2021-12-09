#ifndef RAY_TRACE_H
#define RAY_TRACE_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "light.h"
#include "model.h"

enum Option {
	DEFAULT,
	SOFT_SHADOWS,
	DEPTH_OF_FIELD,
	MOTION_BLUR,
	BUMP_MAPPING
};

glm::vec3 ray_trace(glm::vec3 &p0, glm::vec3 &u, Light *lights[], Model *models[], Images &images,
					bool is_air, int depth, float intensity, Option option);

#endif
