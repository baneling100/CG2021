#include "ray_trace.h"

static glm::vec3 get_texture_color(glm::vec2 &t, unsigned int tex, Images &images)
{
	if (tex) {
		auto &[width, height, image] = images[tex];
		float x = std::fmod(t.x, 1.0f) * width;
		float y = std::fmod(t.y, 1.0f) * height;
		int x0 = x, y0 = y;
		int x1 = (x0 + 1) % width, y1 = (y0 + 1) % height;

		glm::vec3 i00 = glm::vec3(image[y0 * width * 3 + x0 * 3    ] / 255.0f,
								  image[y0 * width * 3 + x0 * 3 + 1] / 255.0f,
								  image[y0 * width * 3 + x0 * 3 + 2] / 255.0f);
		glm::vec3 i10 = glm::vec3(image[y0 * width * 3 + x1 * 3    ] / 255.0f,
								  image[y0 * width * 3 + x1 * 3 + 1] / 255.0f,
								  image[y0 * width * 3 + x1 * 3 + 2] / 255.0f);
		glm::vec3 i01 = glm::vec3(image[y1 * width * 3 + x0 * 3    ] / 255.0f,
								  image[y1 * width * 3 + x0 * 3 + 1] / 255.0f,
								  image[y1 * width * 3 + x0 * 3 + 2] / 255.0f);
		glm::vec3 i11 = glm::vec3(image[y1 * width * 3 + x1 * 3    ] / 255.0f,
								  image[y1 * width * 3 + x1 * 3 + 1] / 255.0f,
								  image[y1 * width * 3 + x1 * 3 + 2] / 255.0f);
		return i00 * (x1 - x) * (y1 - y) + i10 * (x - x0) * (y1 - y) +
			   i01 * (x1 - x) * (y - y0) + i11 * (x - x0) * (y - y0);
	}
	return glm::vec3(1.0f, 1.0f, 1.0f);
}

glm::vec3 ray_trace(glm::vec3 &p0, glm::vec3 &u, Light *lights[], Model *models[], Images &images,
					bool is_air, int depth, float intensity, Option option)
{
	float min_d = 999999999.0f;
	Point pnt;
	Material *mat;
	unsigned int tex;
	bool is_background = true;

	for (int i = 1; i < 10; i++) {
		auto [di, pi, mi, ti, b] = models[i]->nearest_intersect(p0, u);
		if (b && min_d > di) {
			is_background = false;
			min_d = di;
			pnt = pi;
			mat = mi;
			tex = ti;
		}
	}

	if (is_background) {
		auto [di, pi, mi, ti, b] = models[0]->nearest_intersect(p0, u);
		auto [p, N, t] = pi;
		return get_texture_color(t, ti, images);
	}

	auto [p, N, t] = pnt;
	N = glm::normalize(N);
	glm::vec3 texture_color = get_texture_color(t, tex, images);
	glm::vec3 Ka = glm::vec3(mat->ambient[0], mat->ambient[1], mat->ambient[2]);
	glm::vec3 Kd = glm::vec3(mat->diffuse[0], mat->diffuse[1], mat->diffuse[2]);
	glm::vec3 Ks = glm::vec3(mat->specular[0], mat->specular[1], mat->specular[2]);
	float alpha = mat->ambient[3], reflect = mat->reflect, n = mat->shininess;
	// mat->ambient[3] == mat->diffuse[3] == mat->specular[3]
	glm::vec3 V = -u;
	glm::vec3 I = glm::vec3(0.0f, 0.0f, 0.0f);
	if (glm::dot(V, N) < 0.0f) N = -N;
	if (intensity * alpha > 0.01f)
		for (int i = 0; i < 3; i++) {
			glm::vec3 L = glm::normalize(glm::vec3(lights[i]->position[0], lights[i]->position[1], lights[i]->position[2]));
			glm::vec3 R = glm::normalize(2.0f * glm::dot(L, N) * N - L);
			glm::vec3 Ia = glm::vec3(lights[i]->ambient[0], lights[i]->ambient[1], lights[i]->ambient[2]);
			glm::vec3 Id = glm::vec3(lights[i]->diffuse[0], lights[i]->diffuse[1], lights[i]->diffuse[2]);
			glm::vec3 Is = glm::vec3(lights[i]->specular[0], lights[i]->specular[1], lights[i]->specular[2]);
			float SA = 1.0f;
			for (int j = 1; j < 10; j++) {
				SA *= models[j]->shadow_attenuation(p, L);
				if (SA == 0.0f) break;
			}
			I += ((Ka * Ia + (1.0f - reflect) * SA * Kd * Id * std::max(glm::dot(N, L), 0.0f)) * texture_color +
				   SA * Ks * Is * std::pow(std::max(glm::dot(V, R), 0.0f), n)) * intensity * alpha;
		}
	if (intensity * alpha * reflect > 0.01f && depth < 10) {
		glm::vec3 dir = glm::normalize(2.0f * glm::dot(V, N) * N - V);
		I += ray_trace(p, dir, lights, models, images, is_air, depth + 1, intensity * alpha * reflect, option);
	}
	if (intensity * (1.0f - alpha) > 0.01f && depth < 10) {
		float cosi = glm::dot(V, N), ref_coef = is_air ? 0.666f : 1.5f;
		float cosr = std::sqrt(1 - ref_coef * ref_coef * (1 - cosi * cosi));
		glm::vec3 dir = glm::normalize((ref_coef * cosi - cosr) * N - ref_coef * V);
		I += ray_trace(p, dir, lights, models, images, !is_air, depth + 1, intensity * (1.0f - alpha), option);
	}

	return I;
}
