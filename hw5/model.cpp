#include "model.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void parse_stl(std::string &&file_path, Points &points, Faces &faces, Material *material)
{
	FILE *fp = fopen(file_path.c_str(), "rb");
	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);
	rewind(fp);
	char *buffer = (char *)malloc(len * sizeof(char));
	if (!fread(buffer, len, 1, fp)) {
		printf("Error while reading STL file\n");
		exit(0);
	}
	fclose(fp);

	std::map<glm::vec3, int, PointCompare> point_map;
	int cnt = 0;
	char *buf_ptr = buffer + 80;
	unsigned int num_triangles = ((ParsingMask *)buf_ptr)->i;
	buf_ptr += 4;
	for (unsigned int i = 0; i < num_triangles; i++) {
		glm::vec3 n  = glm::vec3(((ParsingMask *) buf_ptr      )->f,
								 ((ParsingMask *)(buf_ptr + 4 ))->f,
								 ((ParsingMask *)(buf_ptr + 8 ))->f);
		glm::vec3 p1 = glm::vec3(((ParsingMask *)(buf_ptr + 12))->f,
								 ((ParsingMask *)(buf_ptr + 16))->f,
								 ((ParsingMask *)(buf_ptr + 20))->f);
		glm::vec3 p2 = glm::vec3(((ParsingMask *)(buf_ptr + 24))->f,
								 ((ParsingMask *)(buf_ptr + 28))->f,
								 ((ParsingMask *)(buf_ptr + 32))->f);
		glm::vec3 p3 = glm::vec3(((ParsingMask *)(buf_ptr + 36))->f,
								 ((ParsingMask *)(buf_ptr + 40))->f,
								 ((ParsingMask *)(buf_ptr + 44))->f);
		buf_ptr += 50;
		
		if (glm::dot(n, n) == 0.0f)
			n = -glm::cross(p2 - p1, p3 - p1);
		
		int idx1, idx2, idx3;
		if (point_map.find(p1) == point_map.end()) {
			idx1 = cnt++;
			point_map[p1] = idx1;
			points.emplace_back(p1, n, glm::vec2(0.0f, 0.0f));
		}
		else {
			idx1 = point_map[p1];
			auto &[p0, n0, t0] = points[idx1];
			n0 += n;
		}
		if (point_map.find(p2) == point_map.end()) {
			idx2 = cnt++;
			point_map[p2] = idx2;
			points.emplace_back(p2, n, glm::vec2(0.0f, 0.0f));
		}
		else {
			idx2 = point_map[p2];
			auto &[p0, n0, t0] = points[idx2];
			n0 += n;
		}
		if (point_map.find(p3) == point_map.end()) {
			idx3 = cnt++;
			point_map[p3] = idx3;
			points.emplace_back(p3, n, glm::vec2(0.0f, 0.0f));
		}
		else {
			idx3 = point_map[p3];
			auto &[p0, n0, t0] = points[idx3];
			n0 += n;
		}
		faces.emplace_back(idx1, idx2, idx3, material, 0);
	}

	for (auto &[p, n, t] : points) n = glm::normalize(n);

	free(buffer);
}

void collect_part(Points &global_points, Faces &global_faces, Points &local_points, Faces &local_faces)
{
	int offset = global_points.size();
	
	for (auto &local_point : local_points)
		global_points.emplace_back(local_point);
	
	for (auto &[x, y, z, m, t] : local_faces)
		global_faces.emplace_back(std::make_tuple(x + offset, y + offset, z + offset, m, t));
}

Model &PokeBall::parse()
{
	parse_stl("models/pokeball/button.stl", button_points, button_faces, &white_plastic);
	parse_stl("models/pokeball/inner_shell.stl", inner_shell_points, inner_shell_faces, &black_plastic);
	parse_stl("models/pokeball/red_lid.stl", red_lid_points, red_lid_faces, &red_plastic);
	parse_stl("models/pokeball/white_lid.stl", white_lid_points, white_lid_faces, &white_plastic_trans);

	for (auto &[p, n, t] : inner_shell_points)
		p += glm::vec3(-80.7f, -45.1f, 3.0f);

	glm::quat quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PIf32, glm::vec3(0.0f, 0.0f, 1.0f));
	for (auto &[p, n, t] : inner_shell_points) {
		p = quat * p;
		n = quat * n;
	}

	for (auto &[p, n, t] : button_points)
		p += glm::vec3(-130.9f, 0.0f, -95.1f);
	for (auto &[p, n, t] : inner_shell_points)
		p += glm::vec3(-130.9f, 0.0f, -95.1f);
	for (auto &[p, n, t] : red_lid_points)
		p += glm::vec3(-130.9f, 0.0f, -95.1f);
	for (auto &[p, n, t] : white_lid_points)
		p += glm::vec3(-130.9f, 0.0f, -95.1f);
	
	return *this;
}

void PokeBall::collect(Points &opaque_points, Faces &opaque_faces,
					   Points &trans_points, Faces &trans_faces)
{
	collect_part(opaque_points, opaque_faces, button_points, button_faces);
	collect_part(opaque_points, opaque_faces, inner_shell_points, inner_shell_faces);
	collect_part(opaque_points, opaque_faces, red_lid_points, red_lid_faces);
	collect_part(trans_points, trans_faces, white_lid_points, white_lid_faces);
}

Model &MasterBall::parse()
{
	parse_stl("models/masterball/button.stl", button_points, button_faces, &white_plastic);
	parse_stl("models/masterball/inner_shell.stl", inner_shell_points, inner_shell_faces, &black_plastic);
	parse_stl("models/masterball/lid_base.stl", lid_base_points, lid_base_faces, &purple_plastic);
	parse_stl("models/masterball/white_lid.stl", white_lid_points, white_lid_faces, &white_plastic_trans);
	parse_stl("models/masterball/m_letter.stl", m_letter_points, m_letter_faces, &white_plastic);
	parse_stl("models/masterball/bump_1.stl", bump_1_points, bump_1_faces, &pink_plastic);
	parse_stl("models/masterball/bump_2.stl", bump_2_points, bump_2_faces, &pink_plastic);

	for (auto &[p, n, t] : inner_shell_points)
		p += glm::vec3(-80.7f, -45.1f, 3.0f);

	glm::quat quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PIf32, glm::vec3(0.0f, 0.0f, 1.0f));
	for (auto &[p, n, t] : inner_shell_points) {
		p = quat * p;
		n = quat * n;
	}

	for (auto &[p, n, t] : lid_base_points)
		p += glm::vec3(-12.5f, 41.0f, 0.0f);
	for (auto &[p, n, t] : bump_1_points)
		p += glm::vec3(-12.5f, 41.0f, 0.0f);
	for (auto &[p, n, t] : bump_2_points)
		p += glm::vec3(-12.5f, 41.0f, 0.0f);
	for (auto &[p, n, t] : m_letter_points)
		p += glm::vec3(-12.5f, 41.0f, 0.0f);

	for (auto &[p, n, t] : button_points)
		p += glm::vec3(-50.0f, 0.0f, -153.9f);
	for (auto &[p, n, t] : inner_shell_points)
		p += glm::vec3(-50.0f, 0.0f, -153.9f);
	for (auto &[p, n, t] : lid_base_points)
		p += glm::vec3(-50.0f, 0.0f, -153.9f);
	for (auto &[p, n, t] : bump_1_points)
		p += glm::vec3(-50.0f, 0.0f, -153.9f);
	for (auto &[p, n, t] : bump_2_points)
		p += glm::vec3(-50.0f, 0.0f, -153.9f);
	for (auto &[p, n, t] : m_letter_points)
		p += glm::vec3(-50.0f, 0.0f, -153.9f);
	for (auto &[p, n, t] : white_lid_points)
		p += glm::vec3(-50.0f, 0.0f, -153.9f);

	return *this;
}

void MasterBall::collect(Points &opaque_points, Faces &opaque_faces,
						 Points &trans_points, Faces &trans_faces)
{
	collect_part(opaque_points, opaque_faces, button_points, button_faces);
	collect_part(opaque_points, opaque_faces, inner_shell_points, inner_shell_faces);
	collect_part(opaque_points, opaque_faces, lid_base_points, lid_base_faces);
	collect_part(opaque_points, opaque_faces, bump_1_points, bump_1_faces);
	collect_part(opaque_points, opaque_faces, bump_2_points, bump_2_faces);
	collect_part(opaque_points, opaque_faces, m_letter_points, m_letter_faces);
	collect_part(trans_points, trans_faces, white_lid_points, white_lid_faces);
}

Model &NetBall::parse()
{
	parse_stl("models/netball/button.stl", button_points, button_faces, &white_plastic);
	parse_stl("models/netball/inner_shell.stl", inner_shell_points, inner_shell_faces, &black_plastic);
	parse_stl("models/netball/lid_base.stl", lid_base_points, lid_base_faces, &cyan_plastic);
	parse_stl("models/netball/white_lid.stl", white_lid_points, white_lid_faces, &white_plastic_trans);
	parse_stl("models/netball/net.stl", net_points, net_faces, &black_plastic);

	for (auto &[p, n, t] : inner_shell_points)
		p += glm::vec3(-80.7f, -45.1f, 3.0f);

	glm::quat quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PIf32, glm::vec3(0.0f, 0.0f, 1.0f));
	for (auto &[p, n, t] : inner_shell_points) {
		p = quat * p;
		n = quat * n;
	}

	for (auto &[p, n, t] : button_points)
		p += glm::vec3(30.9f, 0.0f, -95.1f);
	for (auto &[p, n, t] : inner_shell_points)
		p += glm::vec3(30.9f, 0.0f, -95.1f);
	for (auto &[p, n, t] : net_points)
		p += glm::vec3(30.9f, 0.0f, -98.3f);
	for (auto &[p, n, t] : lid_base_points)
		p += glm::vec3(30.9f, 0.0f, -98.3f);
	for (auto &[p, n, t] : white_lid_points)
		p += glm::vec3(30.9f, 0.0f, -95.1f);

	return *this;
}

void NetBall::collect(Points &opaque_points, Faces &opaque_faces,
					  Points &trans_points, Faces &trans_faces)
{
	collect_part(opaque_points, opaque_faces, button_points, button_faces);
	collect_part(opaque_points, opaque_faces, inner_shell_points, inner_shell_faces);
	collect_part(opaque_points, opaque_faces, net_points, net_faces);
	collect_part(opaque_points, opaque_faces, lid_base_points, lid_base_faces);
	collect_part(trans_points, trans_faces, white_lid_points, white_lid_faces);
}

Model &TimerBall::parse()
{
	parse_stl("models/timerball/button.stl", button_points, button_faces, &white_plastic);
	parse_stl("models/timerball/inner_shell.stl", inner_shell_points, inner_shell_faces, &black_plastic);
	parse_stl("models/timerball/lid_top.stl", lid_top_points, lid_top_faces, &white_plastic);
	parse_stl("models/timerball/lid_bottom.stl", lid_bottom_points, lid_bottom_faces, &white_plastic_trans);
	parse_stl("models/timerball/lid_black.stl", lid_black_points, lid_black_faces, &black_plastic);
	parse_stl("models/timerball/stripes_lower.stl", stripes_lower_points, stripes_lower_faces, &red_plastic);
	parse_stl("models/timerball/stripes_upper.stl", stripes_upper_points, stripes_upper_faces, &red_plastic);
	parse_stl("models/timerball/top_stripe.stl", top_stripe_points, top_stripe_faces, &red_plastic);

	for (auto &[p, n, t] : inner_shell_points)
		p += glm::vec3(-80.7f, -45.1f, 3.0f);

	glm::quat quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PIf32, glm::vec3(0.0f, 0.0f, 1.0f));
	for (auto &[p, n, t] : inner_shell_points) {
		p = quat * p;
		n = quat * n;
	}

	quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PIf32 * 1.0f / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	for (auto &[p, n, t] : top_stripe_points) {
		p = quat * p;
		n = quat * n;
	}

	for (auto &[p, n, t] : button_points)
		p.x -= 100.0f;
	for (auto &[p, n, t] : inner_shell_points)
		p.x -= 100.0f;
	for (auto &[p, n, t] : lid_top_points)
		p.x -= 100.0f;
	for (auto &[p, n, t] : lid_bottom_points)
		p.x -= 100.0f;
	for (auto &[p, n, t] : lid_black_points)
		p.x -= 100.0f;
	for (auto &[p, n, t] : stripes_lower_points)
		p.x -= 100.0f;
	for (auto &[p, n, t] : stripes_upper_points)
		p.x -= 100.0f;
	for (auto &[p, n, t] : top_stripe_points)
		p.x -= 100.0f;

	return *this;
}

void TimerBall::collect(Points &opaque_points, Faces &opaque_faces,
						Points &trans_points, Faces &trans_faces)
{
	collect_part(opaque_points, opaque_faces, button_points, button_faces);
	collect_part(opaque_points, opaque_faces, inner_shell_points, inner_shell_faces);
	collect_part(opaque_points, opaque_faces, lid_top_points, lid_top_faces);
	collect_part(trans_points, trans_faces, lid_bottom_points, lid_bottom_faces);
	collect_part(opaque_points, opaque_faces, lid_black_points, lid_black_faces);
	collect_part(opaque_points, opaque_faces, stripes_lower_points, stripes_lower_faces);
	collect_part(opaque_points, opaque_faces, stripes_upper_points, stripes_upper_faces);
	collect_part(opaque_points, opaque_faces, top_stripe_points, top_stripe_faces);
}

Model &UltraBall::parse()
{
	parse_stl("models/ultraball/button.stl", button_points, button_faces, &white_plastic);
	parse_stl("models/ultraball/inner_shell.stl", inner_shell_points, inner_shell_faces, &black_plastic);
	parse_stl("models/ultraball/lid_black.stl", lid_black_points, lid_black_faces, &black_plastic);
	parse_stl("models/ultraball/white_lid.stl", white_lid_points, white_lid_faces, &white_plastic_trans);
	parse_stl("models/ultraball/lid_yellow.stl", lid_yellow_points, lid_yellow_faces, &yellow_plastic);

	for (auto &[p, n, t] : inner_shell_points)
		p += glm::vec3(-80.7f, -45.1f, 3.0f);

	glm::quat quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PIf32, glm::vec3(0.0f, 0.0f, 1.0f));
	for (auto &[p, n, t] : inner_shell_points) {
		p = quat * p;
		n = quat * n;
	}

	for (auto &[p, n, t] : lid_black_points)
		p.y += 41.0f;
	for (auto &[p, n, t] : lid_yellow_points)
		p.y += 41.0f;

	return *this;
}

void UltraBall::collect(Points &opaque_points, Faces &opaque_faces,
						Points &trans_points, Faces &trans_faces)
{
	collect_part(opaque_points, opaque_faces, button_points, button_faces);
	collect_part(opaque_points, opaque_faces, inner_shell_points, inner_shell_faces);
	collect_part(opaque_points, opaque_faces, lid_black_points, lid_black_faces);
	collect_part(opaque_points, opaque_faces, lid_yellow_points, lid_yellow_faces);
	collect_part(trans_points, trans_faces, white_lid_points, white_lid_faces);
}

Model &Table::parse()
{
	parse_stl("models/table.stl", table_points, table_faces, &green_plastic);

	for (auto &[p, n, t] : table_points)
		p = 7500.0f * p + glm::vec3(0.0f, -250.0f, 0.0f);

	glm::quat quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PIf32 / 3.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	for (auto &[p, n, t] : table_points) {
		p = quat * p;
		n = quat * n;
	}

	return *this;
}

void Table::collect(Points &opaque_points, Faces &opaque_faces,
					Points &trans_points, Faces &trans_faces)
{
	collect_part(opaque_points, opaque_faces, table_points, table_faces);
}

Model &GlassSphere::parse()
{
	int h = 32;
	int w = 2 * h;
	int cnt = 0;
	std::map<glm::vec3, int, PointCompare> point_map;

	for (int i = 1; i < h; i++)
		for (int j = 1; j < w; j++) {
			float theta0 = M_PIf32 * (i - 1) / (h - 1);
			float theta1 = M_PIf32 * i / (h - 1);
			float phi0 = 2.0f * M_PIf32 * (j - 1) / (w - 1) - M_PI_4f32;
			float phi1 = 2.0f * M_PIf32 * j / (w - 1) - M_PI_4f32;
			
			glm::vec3 p1 = glm::vec3(sin(theta0) * cos(phi0), cos(theta0), sin(theta0) * sin(phi0));
			glm::vec3 p2 = glm::vec3(sin(theta1) * cos(phi0), cos(theta1), sin(theta1) * sin(phi0));
			glm::vec3 p3 = glm::vec3(sin(theta1) * cos(phi1), cos(theta1), sin(theta1) * sin(phi1));
			glm::vec3 p4 = glm::vec3(sin(theta0) * cos(phi1), cos(theta0), sin(theta0) * sin(phi1));

			int idx1, idx2, idx3, idx4;
			if (point_map.find(p1) == point_map.end()) {
				idx1 = cnt++;
				point_map[p1] = idx1;
				sphere_points.emplace_back(p1, p1, glm::vec2(0.0f, 0.0f));
			}
			else idx1 = point_map[p1];
			if (point_map.find(p2) == point_map.end()) {
				idx2 = cnt++;
				point_map[p2] = idx2;
				sphere_points.emplace_back(p2, p2, glm::vec2(0.0f, 0.0f));
			}
			else idx2 = point_map[p2];
			if (point_map.find(p3) == point_map.end()) {
				idx3 = cnt++;
				point_map[p3] = idx3;
				sphere_points.emplace_back(p3, p3, glm::vec2(0.0f, 0.0f));
			}
			else idx3 = point_map[p3];
			if (point_map.find(p4) == point_map.end()) {
				idx3 = cnt++;
				point_map[p4] = idx4;
				sphere_points.emplace_back(p4, p4, glm::vec2(0.0f, 0.0f));
			}
			else idx4 = point_map[p4];

			sphere_faces.emplace_back(idx1, idx2, idx3, &white_plastic_trans, 0);
			sphere_faces.emplace_back(idx3, idx4, idx1, &white_plastic_trans, 0);
		}

	for (auto &[p, n, t] : sphere_points)
		p = 40.0f * p + glm::vec3(-7.0f, -51.0f, 85.4f);

	return *this;
}

void GlassSphere::collect(Points &opaque_points, Faces &opaque_faces,
						  Points &trans_points, Faces &trans_faces)
{
	collect_part(trans_points, trans_faces, sphere_points, sphere_faces);
}

Model &WoodenSphere::parse()
{
	int width, height, num_channels;
	unsigned char *image = stbi_load("textures/wood.jpg", &width, &height, &num_channels, 3);

	glGenTextures(1, &wooden_texture);
	glBindTexture(GL_TEXTURE_2D, wooden_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	
	stbi_image_free(image);


	int h = 32;
	int w = h * width / height;
	int cnt = 0;
	std::map<glm::vec3, int, PointCompare> point_map;

	for (int i = 1; i < h; i++)
		for (int j = 1; j < w; j++) {
			float theta0 = M_PIf32 * (i - 1) / (h - 1);
			float theta1 = M_PIf32 * i / (h - 1);
			float phi0 = 2.0f * M_PIf32 * (j - 1) / (w - 1) - M_PI_4f32;
			float phi1 = 2.0f * M_PIf32 * j / (w - 1) - M_PI_4f32;

			glm::vec2 t1 = glm::vec2((float)(j - 1) / (w - 1), (float)(i - 1) / (h - 1));
			glm::vec2 t2 = glm::vec2((float)(j - 1) / (w - 1), (float)i / (h - 1));
			glm::vec2 t3 = glm::vec2((float)j / (w - 1), (float)i / (h - 1));
			glm::vec2 t4 = glm::vec2((float)j / (w - 1), (float)(i - 1) / (h - 1));
			
			glm::vec3 p1 = glm::vec3(sin(theta0) * cos(phi0), cos(theta0), sin(theta0) * sin(phi0));
			glm::vec3 p2 = glm::vec3(sin(theta1) * cos(phi0), cos(theta1), sin(theta1) * sin(phi0));
			glm::vec3 p3 = glm::vec3(sin(theta1) * cos(phi1), cos(theta1), sin(theta1) * sin(phi1));
			glm::vec3 p4 = glm::vec3(sin(theta0) * cos(phi1), cos(theta0), sin(theta0) * sin(phi1));

			int idx1, idx2, idx3, idx4;
			if (point_map.find(p1) == point_map.end()) {
				idx1 = cnt++;
				point_map[p1] = idx1;
				wooden_points.emplace_back(p1, p1, t1);
			}
			else idx1 = point_map[p1];
			if (point_map.find(p2) == point_map.end()) {
				idx2 = cnt++;
				point_map[p2] = idx2;
				wooden_points.emplace_back(p2, p2, t2);
			}
			else idx2 = point_map[p2];
			if (point_map.find(p3) == point_map.end()) {
				idx3 = cnt++;
				point_map[p3] = idx3;
				wooden_points.emplace_back(p3, p3, t3);
			}
			else idx3 = point_map[p3];
			if (point_map.find(p4) == point_map.end()) {
				idx3 = cnt++;
				point_map[p4] = idx4;
				wooden_points.emplace_back(p4, p4, t4);
			}
			else idx4 = point_map[p4];

			wooden_faces.emplace_back(idx1, idx2, idx3, &texture_material, wooden_texture);
			wooden_faces.emplace_back(idx3, idx4, idx1, &texture_material, wooden_texture);
		}

	for (auto &[p, n, t] : wooden_points)
		p = 40.0f * p + glm::vec3(93.7f, -51.0f, -87.7f);

	return *this;
}

void WoodenSphere::collect(Points &opaque_points, Faces &opaque_faces,
						   Points &trans_points, Faces &trans_faces)
{
	collect_part(opaque_points, opaque_faces, wooden_points, wooden_faces);
}

Model &Dice::parse()
{
	int width, height, num_channels;
	unsigned char *image = stbi_load("textures/dice.png", &width, &height, &num_channels, 3);

	glGenTextures(1, &dice_texture);
	glBindTexture(GL_TEXTURE_2D, dice_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	
	stbi_image_free(image);

	dice_points.emplace_back(glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.00f, 0.50f));
	dice_points.emplace_back(glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.00f, 0.75f));
	dice_points.emplace_back(glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.25f, 0.75f));
	dice_points.emplace_back(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.25f, 0.50f));
	dice_points.emplace_back(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.25f, 0.50f));
	dice_points.emplace_back(glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.25f, 0.75f));
	dice_points.emplace_back(glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.50f, 0.75f));
	dice_points.emplace_back(glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.50f, 0.50f));
	dice_points.emplace_back(glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.50f, 0.50f));
	dice_points.emplace_back(glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.50f, 0.75f));
	dice_points.emplace_back(glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.75f, 0.75f));
	dice_points.emplace_back(glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.75f, 0.50f));
	dice_points.emplace_back(glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.75f, 0.50f));
	dice_points.emplace_back(glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.75f, 0.75f));
	dice_points.emplace_back(glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.00f, 0.75f));
	dice_points.emplace_back(glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.00f, 0.50f));
	dice_points.emplace_back(glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.50f, 0.75f));
	dice_points.emplace_back(glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.50f, 1.00f));
	dice_points.emplace_back(glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.75f, 1.00f));
	dice_points.emplace_back(glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.75f, 0.75f));
	dice_points.emplace_back(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.50f, 0.25f));
	dice_points.emplace_back(glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.50f, 0.50f));
	dice_points.emplace_back(glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.75f, 0.50f));
	dice_points.emplace_back(glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.75f, 0.25f));

	dice_faces.emplace_back( 0,  1,  2, &texture_material, dice_texture);
	dice_faces.emplace_back( 2,  3,  0, &texture_material, dice_texture);
	dice_faces.emplace_back( 4,  5,  6, &texture_material, dice_texture);
	dice_faces.emplace_back( 6,  7,  4, &texture_material, dice_texture);
	dice_faces.emplace_back( 8,  9, 10, &texture_material, dice_texture);
	dice_faces.emplace_back(10, 11,  8, &texture_material, dice_texture);
	dice_faces.emplace_back(12, 13, 14, &texture_material, dice_texture);
	dice_faces.emplace_back(14, 15, 12, &texture_material, dice_texture);
	dice_faces.emplace_back(16, 17, 18, &texture_material, dice_texture);
	dice_faces.emplace_back(18, 19, 16, &texture_material, dice_texture);
	dice_faces.emplace_back(20, 21, 22, &texture_material, dice_texture);
	dice_faces.emplace_back(22, 23, 20, &texture_material, dice_texture);

	glm::quat quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PI_4f32, glm::vec3(1.0f, 0.0f, 0.0f));
	quat = glm::rotate(quat, M_PI_4f32, glm::vec3(0.0f, 0.0f, 1.0f));
	for (auto &[p, n, t] : dice_points) {
		p = 30.0f * (quat * p) + glm::vec3(-107.0f, -39.0f, -87.7f);
		n = quat * n;
	}

	return *this;
}

void Dice::collect(Points &opaque_points, Faces &opaque_faces,
				   Points &trans_points, Faces &trans_faces)
{
	collect_part(opaque_points, opaque_faces, dice_points, dice_faces);
}

Model &Background::parse()
{
	int width, height, num_channels;
	unsigned char *image = stbi_load("textures/background.jpg", &width, &height, &num_channels, 3);

	glGenTextures(1, &background_texture);
	glBindTexture(GL_TEXTURE_2D, background_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	
	stbi_image_free(image);


	int h = 32;
	int w = h * width / height;
	int cnt = 0;
	std::map<glm::vec3, int, PointCompare> point_map;

	for (int i = 1; i < h; i++)
		for (int j = 1; j < w; j++) {
			float theta0 = M_PIf32 * (i - 1) / (h - 1);
			float theta1 = M_PIf32 * i / (h - 1);
			float phi0 = 2.0f * M_PIf32 * (j - 1) / (w - 1) - M_PI_4f32;
			float phi1 = 2.0f * M_PIf32 * j / (w - 1) - M_PI_4f32;

			glm::vec2 t1 = glm::vec2((float)(j - 1) / (w - 1), (float)(i - 1) / (h - 1));
			glm::vec2 t2 = glm::vec2((float)(j - 1) / (w - 1), (float)i / (h - 1));
			glm::vec2 t3 = glm::vec2((float)j / (w - 1), (float)i / (h - 1));
			glm::vec2 t4 = glm::vec2((float)j / (w - 1), (float)(i - 1) / (h - 1));
			
			glm::vec3 p1 = glm::vec3(sin(theta0) * cos(phi0), cos(theta0), sin(theta0) * sin(phi0));
			glm::vec3 p2 = glm::vec3(sin(theta1) * cos(phi0), cos(theta1), sin(theta1) * sin(phi0));
			glm::vec3 p3 = glm::vec3(sin(theta1) * cos(phi1), cos(theta1), sin(theta1) * sin(phi1));
			glm::vec3 p4 = glm::vec3(sin(theta0) * cos(phi1), cos(theta0), sin(theta0) * sin(phi1));

			int idx1, idx2, idx3, idx4;
			if (point_map.find(p1) == point_map.end()) {
				idx1 = cnt++;
				point_map[p1] = idx1;
				background_points.emplace_back(p1, p1, t1);
			}
			else idx1 = point_map[p1];
			if (point_map.find(p2) == point_map.end()) {
				idx2 = cnt++;
				point_map[p2] = idx2;
				background_points.emplace_back(p2, p2, t2);
			}
			else idx2 = point_map[p2];
			if (point_map.find(p3) == point_map.end()) {
				idx3 = cnt++;
				point_map[p3] = idx3;
				background_points.emplace_back(p3, p3, t3);
			}
			else idx3 = point_map[p3];
			if (point_map.find(p4) == point_map.end()) {
				idx3 = cnt++;
				point_map[p4] = idx4;
				background_points.emplace_back(p4, p4, t4);
			}
			else idx4 = point_map[p4];

			background_faces.emplace_back(idx1, idx2, idx3, &texture_material, background_texture);
			background_faces.emplace_back(idx3, idx4, idx1, &texture_material, background_texture);
		}

	for (auto &[p, n, t] : background_points)
		p = 1250.0f * p;

	return *this;
}

void Background::collect(Points &opaque_points, Faces &opaque_faces,
						 Points &trans_points, Faces &trans_faces)
{
	collect_part(opaque_points, opaque_faces, background_points, background_faces);
}
