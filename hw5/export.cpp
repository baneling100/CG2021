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

std::mutex m;
int x, y;

void worker(unsigned char *image, int width, int height, float fovy, glm::vec3 camera,
			glm::vec3 origin, glm::vec3 axisY, Light *lights[], Model *models[], Images images)
{
	while (y < height) {
		m.lock();
		int j = x, i = y;
		x++;
		if (x == width) {
			y++;
			x = 0;
			printf("\r%d / %d", y, height);
			fflush(stdout);
		}
		m.unlock();
		glm::vec3 p0 = glm::unProject(glm::vec3((float)j, (float)(height - i - 1), 0.1f),
						glm::lookAt(camera, origin, axisY),
						glm::perspective(glm::radians(fovy), (float)width / height, 0.1f, 3000.0f),
						glm::vec4(0.0f, 0.0f, (float)width, (float)height));
		glm::vec3 u = glm::normalize(p0 - camera);
		glm::vec3 color = ray_trace(p0, u, lights, models, images, 0, true);
		image[i * width * 3 + j * 3    ] = 255.0f * color.x;
		image[i * width * 3 + j * 3 + 1] = 255.0f * color.y;
		image[i * width * 3 + j * 3 + 2] = 255.0f * color.z;
	}
}

void export_raytraced_image(std::string &name, int width, int height, float fovy, glm::vec3 &camera,
glm::vec3 &origin, glm::vec3 &axisY, Light *lights[], Model *models[], Images &images, int num_threads)
{
	unsigned char *image = (unsigned char *)malloc(height * width * 3);
	std::vector<std::thread> threads;
	x = y = 0;

	printf("0 / %d", height);
	fflush(stdout);
	for (int i = 0; i < num_threads; i++)
		threads.emplace_back(std::thread(worker, image, width, height, fovy, camera,
												 origin, axisY, lights, models, images));
	for (int i = 0; i < num_threads; i++)
		threads[i].join();
	printf("\n");
	stbi_write_bmp(name.c_str(), width, height, 3, image);
	free(image);
}

void export_soft_shadows_image(std::string &name, int width, int height, float fovy, glm::vec3 &camera,
				glm::vec3 &origin, glm::vec3 &axisY, Light *lights[], Model *models[], Images &images, int num_threads)
{
	// unsigned char *image = (unsigned char *)malloc(height * width * 3);
	// int num_rays = 8;

	// std::srand(std::time(NULL));

	// for (int i = 0; i < height; i++)
	// 	for (int j = 0; j < width; j++) {
	// 		glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);
	// 		for (int k = 0; k < num_rays; k++) {
	// 			float x = j + (float)std::rand() / RAND_MAX - 0.5f;
	// 			float y = height - i - 1 + (float)std::rand() / RAND_MAX - 0.5f;
	// 			glm::vec3 p0 = glm::unProject(glm::vec3(x, y, 0.1f),
	// 						   glm::lookAt(camera, origin, axisY),
	// 						   glm::perspective(glm::radians(fovy), (float)width / height, 0.1f, 3000.0f),
	// 						   glm::vec4(0.0f, 0.0f, (float)width, (float)height));
	// 			glm::vec3 u = p0 - camera;
	// 			color += ray_trace(p0, u, lights, models, images, 0, true);
	// 		}
	// 		color /= num_rays;
	// 		image[i * width * 3 + j    ] = 255.0f * color.x;
	// 		image[i * width * 3 + j + 1] = 255.0f * color.y;
	// 		image[i * width * 3 + j + 2] = 255.0f * color.z;
	// 	}
	// stbi_write_png(name.c_str(), width, height, 3, image, width * 3);
	// free(image);
}

void export_depth_of_field_image(std::string &name, int width, int height, float fovy, glm::vec3 &camera,
				glm::vec3 &origin, glm::vec3 &axisY, Light *lights[], Model *models[], Images &images, int num_threads)
{
}

void export_motion_blur_image(std::string &name, int width, int height, float fovy, glm::vec3 &camera,
				glm::vec3 &origin, glm::vec3 &axisY, Light *lights[], Model *models[], Images &images, int num_threads)
{
}

void export_bump_mapping_image(std::string &name, int width, int height, float fovy, glm::vec3 &camera,
				glm::vec3 &origin, glm::vec3 &axisY, Light *lights[], Model *models[], Images &images, int num_threads)
{
}
