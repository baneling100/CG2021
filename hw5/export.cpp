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
	printf("%d / %d\n", height, height);
	stbi_write_png(name.c_str(), width, height, 3, image, width * 3);
	free(image);
	free(line);
}

void worker(std::mutex *m, int *x, int *y,
	unsigned char *image, int width, int height, float fovy, glm::vec3 camera, glm::vec3 origin,
	glm::vec3 axisY, Light *lights[], Model *models[], Images images, Option option)
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
			printf("\r%d / %d", (*y), height);
			fflush(stdout);
		}
		m->unlock();
		glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);
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
		color /= 8;
		image[i * width * 3 + j * 3    ] = (unsigned char)(255.0f * std::min(color.x, 1.0f));
		image[i * width * 3 + j * 3 + 1] = (unsigned char)(255.0f * std::min(color.y, 1.0f));
		image[i * width * 3 + j * 3 + 2] = (unsigned char)(255.0f * std::min(color.z, 1.0f));
	}
}

void export_raytraced_image(std::string &name, int width, int height, float fovy, glm::vec3 &camera,
glm::vec3 &origin, glm::vec3 &axisY, Light *lights[], Model *models[], Images &images, int num_threads, Option option)
{
	unsigned char *image = (unsigned char *)malloc(height * width * 3);
	std::vector<std::thread> threads;
	std::mutex m;
	int x = 0, y = 0;

	printf("0 / %d", height);
	fflush(stdout);
	for (int i = 0; i < num_threads; i++)
		threads.emplace_back(std::thread(worker, &m, &x, &y, image, width, height, fovy, camera,
												 origin, axisY, lights, models, images, option));
	for (int i = 0; i < num_threads; i++)
		threads[i].join();
	printf("\n");
	stbi_write_bmp(name.c_str(), width, height, 3, image);
	free(image);
}
