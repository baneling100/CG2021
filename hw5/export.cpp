#include "export.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void export_opengl_image(std::string &name, int width, int height)
{
	unsigned char *image = (unsigned char *)malloc(height * width * 3);
	unsigned char *line = (unsigned char *)malloc(width * 3);
	
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
	for (int i = 0; i < height / 2; i++) {
		memcpy(line, image + i * width * 3, width * 3);
		memcpy(image + i * width * 3, image + (height - i - 1) * width * 3, width * 3);
		memcpy(image + (height - i - 1) * width * 3, line, width * 3);
	}
	stbi_write_png(name.c_str(), width, height, 3, image, width * 3);
	free(image);
	free(line);
	printf("100 %%\n%s is saved\n", name.c_str());
}

void worker(std::mutex *m, int *x, int *y,
	unsigned char *image, int width, int height, float fovy, glm::vec3 camera, glm::vec3 origin,
	glm::vec3 axisX, glm::vec3 axisY, Light *lights[], Model *models[], Images images, Option option)
{
	std::srand(std::time(NULL));
	glm::vec3 p1 = glm::unProject(glm::vec3(width / 2.0f, height / 2.0f, 0.001f),
					glm::lookAt(camera, origin, axisY),
					glm::perspective(glm::radians(fovy), (float)width / height, 0.001f, 30.0f),
					glm::vec4(0.0f, 0.0f, (float)width, (float)height));
	float d1 = glm::distance(camera, p1);
	float d2 = glm::distance(camera, origin);
	while (true) {
		m->lock();
		if (*y == height) {
			m->unlock();
			break;
		}
		int j = *x, i = *y;
		(*x)++;
		if ((*x) == width) {
			(*y)++;
			(*x) = 0;
			printf("\r%d %%", 100 * (*y) / height);
			fflush(stdout);
		}
		m->unlock();
		glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);
		if (option == DEPTH_OF_FIELD) {
			float tx = j + 0.5f;
			float ty = height - i - 0.5f;
			glm::vec3 p0 = glm::unProject(glm::vec3(tx, ty, 0.001f),
						glm::lookAt(camera, origin, axisY),
						glm::perspective(glm::radians(fovy), (float)width / height, 0.001f, 30.0f),
						glm::vec4(0.0f, 0.0f, (float)width, (float)height));
			glm::vec3 f0 = camera + d2 * (p0 - camera) / d1;
			for (int dy = -2; dy <= 2; dy++)
				for (int dx = -2; dx <= 2; dx++) {
					glm::vec3 tcam = camera + dy * 0.02f * axisY + dx * 0.02f * axisX;
					glm::vec3 u = glm::normalize(f0 - tcam);
					color += ray_trace(tcam, u, lights, models, images, true, 0, 1.0f, option);
				}
			color /= 25.0f;
		}
		else {
			for (int k = 0; k < 8; k++) {
				float tx = j + (float)std::rand() / RAND_MAX;
				float ty = height - i + (float)std::rand() / RAND_MAX - 1.0f;
				glm::vec3 p0 = glm::unProject(glm::vec3(tx, ty, 0.001f),
								glm::lookAt(camera, origin, axisY),
								glm::perspective(glm::radians(fovy), (float)width / height, 0.001f, 30.0f),
								glm::vec4(0.0f, 0.0f, (float)width, (float)height));
				glm::vec3 u = glm::normalize(p0 - camera);
				color += ray_trace(camera, u, lights, models, images, true, 0, 1.0f, option);
			}
			color /= 8.0f;
		}
		image[i * width * 3 + j * 3    ] = (unsigned char)(255.0f * std::min(color.x, 1.0f));
		image[i * width * 3 + j * 3 + 1] = (unsigned char)(255.0f * std::min(color.y, 1.0f));
		image[i * width * 3 + j * 3 + 2] = (unsigned char)(255.0f * std::min(color.z, 1.0f));
	}
}

void motion_worker(std::mutex *m, int *x, int *y, int step,
	glm::vec3 *buffer, int width, int height, float fovy, glm::vec3 camera, glm::vec3 origin,
	glm::vec3 axisX, glm::vec3 axisY, Light *lights[], Model *models[], Images images, Option option)
{
	std::srand(std::time(NULL));
	while (true) {
		m->lock();
		if (*y == height) {
			m->unlock();
			break;
		}
		int j = *x, i = *y;
		(*x)++;
		if ((*x) == width) {
			(*y)++;
			(*x) = 0;
			printf("\r%d %%", 25 * (step * height + (*y)) / (2 * height));
			fflush(stdout);
		}
		m->unlock();
		float tx = j + 0.5f;
		float ty = height - i - 0.5f;
		glm::vec3 p0 = glm::unProject(glm::vec3(tx, ty, 0.001f),
						glm::lookAt(camera, origin, axisY),
						glm::perspective(glm::radians(fovy), (float)width / height, 0.001f, 30.0f),
						glm::vec4(0.0f, 0.0f, (float)width, (float)height));
		glm::vec3 u = glm::normalize(p0 - camera);
		glm::vec3 color = ray_trace(camera, u, lights, models, images, true, 0, 1.0f, option);
		buffer[i * width + j] += glm::vec3(std::min(color.x, 1.0f), std::min(color.y, 1.0f), std::min(color.z, 1.0f));
	}
}

void export_raytraced_image(std::string &name, int width, int height, float fovy, glm::vec3 &camera,
		glm::vec3 &origin, glm::vec3 &axisX, glm::vec3 &axisY, Light *lights[], Model *models[],
		Images &images, int num_threads, Option option)
{
	unsigned char *image = (unsigned char *)malloc(height * width * 3);
	std::vector<std::thread> threads;
	std::mutex m;
	int x = 0, y = 0;

	if (option == MOTION_BLUR) {
		glm::vec3 *buffer = (glm::vec3 *)malloc(height * width * sizeof(glm::vec3));
		for (int i = 0; i < height; i++)
			for (int j = 0; j < width; j++)
				buffer[i * width + j] = glm::vec3(0.0f, 0.0f, 0.0f);
		for (int k = 0; k < 8; k++) {
			models[5]->set_motion(k * 0.01f * glm::vec3(-1.0f, 0.0f, 0.0f));
			models[6]->set_motion(k * 0.01f * glm::vec3(0.0f, 0.0f, 1.0f));
			models[7]->set_motion(k * 0.01f * glm::vec3(1.0f, 0.0f, 0.0f));
			models[8]->set_motion(k * 0.01f * glm::vec3(0.0f, 0.0f, 1.0f));
			models[9]->set_motion(k * 0.01f * glm::vec3(0.0f, 1.0f, 0.0f));
			x = 0; y = 0;
			printf("\r%d %%", 25 * k * height / (2 * height));
			fflush(stdout);
			threads.clear();
			for (int i = 0; i < num_threads; i++)
				threads.emplace_back(std::thread(motion_worker, &m, &x, &y, k, buffer, width, height, fovy, camera,
												 origin, axisX, axisY, lights, models, images, option));
			for (int i = 0; i < num_threads; i++)
				threads[i].join();
		}
		models[5]->set_motion(glm::vec3(0.0f, 0.0f, 0.0f));
		models[6]->set_motion(glm::vec3(0.0f, 0.0f, 0.0f));
		models[7]->set_motion(glm::vec3(0.0f, 0.0f, 0.0f));
		models[8]->set_motion(glm::vec3(0.0f, 0.0f, 0.0f));
		models[9]->set_motion(glm::vec3(0.0f, 0.0f, 0.0f));
		for (int i = 0; i < height; i++)
			for (int j = 0; j < width; j++) {
				buffer[i * width + j] /= 8.0f;
				image[i * width * 3 + j * 3    ] = (unsigned char)(255.0f * buffer[i * width + j].x);
				image[i * width * 3 + j * 3 + 1] = (unsigned char)(255.0f * buffer[i * width + j].y);
				image[i * width * 3 + j * 3 + 2] = (unsigned char)(255.0f * buffer[i * width + j].z);
			}
		free(buffer);
	}
	else {
		printf("0 %%");
		fflush(stdout);
		for (int i = 0; i < num_threads; i++)
			threads.emplace_back(std::thread(worker, &m, &x, &y, image, width, height, fovy, camera,
												origin, axisX, axisY, lights, models, images, option));
		for (int i = 0; i < num_threads; i++)
			threads[i].join();
	}
	stbi_write_bmp(name.c_str(), width, height, 3, image);
	free(image);
	printf("\n%s is saved\n", name.c_str());
}
