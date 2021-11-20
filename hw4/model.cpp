#include "model.h"

void Model::parse_parts(std::string &&file_path, Model::Points &points, Model::Faces &faces,
						Material *material)
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

	std::map<glm::vec3, int, Comparator> point_map;
	int cnt = 0;
	char *buf_ptr = buffer + 80;
	unsigned int num_triangles = ((Convert *)buf_ptr)->i;
	buf_ptr += 4;
	for (unsigned int i = 0; i < num_triangles; i++) {
		glm::vec3 n  = glm::vec3(((Convert *) buf_ptr      )->f,
								 ((Convert *)(buf_ptr + 4 ))->f,
								 ((Convert *)(buf_ptr + 8 ))->f);
		glm::vec3 p1 = glm::vec3(((Convert *)(buf_ptr + 12))->f,
								 ((Convert *)(buf_ptr + 16))->f,
								 ((Convert *)(buf_ptr + 20))->f);
		glm::vec3 p2 = glm::vec3(((Convert *)(buf_ptr + 24))->f,
								 ((Convert *)(buf_ptr + 28))->f,
								 ((Convert *)(buf_ptr + 32))->f);
		glm::vec3 p3 = glm::vec3(((Convert *)(buf_ptr + 36))->f,
								 ((Convert *)(buf_ptr + 40))->f,
								 ((Convert *)(buf_ptr + 44))->f);
		buf_ptr += 50;
		
		if (glm::dot(n, n) == 0.0f)
			n = glm::cross(p2 - p1, p3 - p1);
		
		int idx1, idx2, idx3;
		if (point_map.find(p1) == point_map.end()) {
			idx1 = cnt++;
			point_map[p1] = idx1;
			points.emplace_back(p1, n);
		}
		else {
			idx1 = point_map[p1];
			points[idx1].second += n;
		}
		if (point_map.find(p2) == point_map.end()) {
			idx2 = cnt++;
			point_map[p2] = idx2;
			points.emplace_back(p2, n);
		}
		else {
			idx2 = point_map[p2];
			points[idx2].second += n;
		}
		if (point_map.find(p3) == point_map.end()) {
			idx3 = cnt++;
			point_map[p3] = idx3;
			points.emplace_back(p3, n);
		}
		else {
			idx3 = point_map[p3];
			points[idx3].second += n;
		}
		faces.emplace_back(idx1, idx2, idx3, material);
	}

	int size = points.size();
	for (int i = 0; i < size; i++)
		points[i].second = glm::normalize(points[i].second);

	free(buffer);
}

void Model::collect_parts(Model::Points &global_points, Model::Faces &global_faces,
						  Model::Points &local_points, Model::Faces &local_faces)
{
	int offset = global_points.size();
	
	for (auto &local_point : local_points)
		global_points.emplace_back(local_point);
	
	for (auto &[x, y, z, m] : local_faces)
		global_faces.emplace_back(std::make_tuple(x + offset, y + offset, z + offset, m));
}

Model &PokeBall::parse()
{
	parse_parts("models/pokeball/button.stl", button_points, button_faces, &white_plastic);
	parse_parts("models/pokeball/inner_shell.stl", inner_shell_points, inner_shell_faces,
				&black_plastic);
	parse_parts("models/pokeball/red_lid.stl", red_lid_points, red_lid_faces, &red_plastic);
	parse_parts("models/pokeball/white_lid.stl", white_lid_points, white_lid_faces, &white_plastic_trans);

	return *this;
}

Model &PokeBall::collect(Points &opaque_points, Faces &opaque_faces,
						 Points &trans_points, Faces &trans_faces)
{
	for (auto &[p, q] : inner_shell_points)
		p = glm::vec3(p.x - 80.8f, p.y - 45.0f, p.z + 3.0f);

	glm::quat quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PIf32, glm::vec3(0.0f, 0.0f, 1.0f));
	for (auto &point : inner_shell_points) {
		glm::vec4 p = quat * glm::vec4(point.first, 1.0f);
		glm::vec4 q = quat * glm::vec4(point.second, 1.0f);
		point = std::make_pair(glm::vec3(p.x, p.y, p.z), glm::vec3(q.x, q.y, q.z));
	}

	for (auto &[p, q] : button_points)
		p = glm::vec3(p.x - 150.0f, p.y, p.z - 100.0f);
	for (auto &[p, q] : inner_shell_points)
		p = glm::vec3(p.x - 150.0f, p.y, p.z - 100.0f);
	for (auto &[p, q] : red_lid_points)
		p = glm::vec3(p.x - 150.0f, p.y, p.z - 100.0f);
	for (auto &[p, q] : white_lid_points)
		p = glm::vec3(p.x - 150.0f, p.y, p.z - 100.0f);

	collect_parts(opaque_points, opaque_faces, button_points, button_faces);
	collect_parts(opaque_points, opaque_faces, inner_shell_points, inner_shell_faces);
	collect_parts(opaque_points, opaque_faces, red_lid_points, red_lid_faces);
	collect_parts(trans_points, trans_faces, white_lid_points, white_lid_faces);

	return *this;
}

Model &MasterBall::parse()
{
	parse_parts("models/masterball/button.stl", button_points, button_faces, &white_plastic);
	parse_parts("models/masterball/inner_shell.stl", inner_shell_points, inner_shell_faces,
				&black_plastic);
	parse_parts("models/masterball/lid_base.stl", lid_base_points, lid_base_faces, &purple_plastic);
	parse_parts("models/masterball/white_lid.stl", white_lid_points, white_lid_faces, &white_plastic_trans);
	parse_parts("models/masterball/m_letter.stl", m_letter_points, m_letter_faces, &white_plastic);
	parse_parts("models/masterball/bump_1.stl", bump_1_points, bump_1_faces, &pink_plastic);
	parse_parts("models/masterball/bump_2.stl", bump_2_points, bump_2_faces, &pink_plastic);

	return *this;
}

Model &MasterBall::collect(Points &opaque_points, Faces &opaque_faces,
						   Points &trans_points, Faces &trans_faces)
{
	for (auto &[p, q] : inner_shell_points)
		p = glm::vec3(p.x - 80.8f, p.y - 45.0f, p.z + 3.0f);

	glm::quat quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PIf32, glm::vec3(0.0f, 0.0f, 1.0f));
	for (auto &point : inner_shell_points) {
		glm::vec4 p = quat * glm::vec4(point.first, 1.0f);
		glm::vec4 q = quat * glm::vec4(point.second, 1.0f);
		point = std::make_pair(glm::vec3(p.x, p.y, p.z), glm::vec3(q.x, q.y, q.z));
	}

	for (auto &[p, q] : lid_base_points)
		p = glm::vec3(p.x - 12.5f, p.y + 41.0f, p.z);
	for (auto &[p, q] : bump_1_points)
		p = glm::vec3(p.x - 12.5f, p.y + 41.0f, p.z);
	for (auto &[p, q] : bump_2_points)
		p = glm::vec3(p.x - 12.5f, p.y + 41.0f, p.z);
	for (auto &[p, q] : m_letter_points)
		p = glm::vec3(p.x - 12.5f, p.y + 41.0f, p.z);

	for (auto &[p, q] : button_points)
		p = glm::vec3(p.x - 50.0f, p.y, p.z - 100.0f);
	for (auto &[p, q] : inner_shell_points)
		p = glm::vec3(p.x - 50.0f, p.y, p.z - 100.0f);
	for (auto &[p, q] : lid_base_points)
		p = glm::vec3(p.x - 50.0f, p.y, p.z - 100.0f);
	for (auto &[p, q] : bump_1_points)
		p = glm::vec3(p.x - 50.0f, p.y, p.z - 100.0f);
	for (auto &[p, q] : bump_2_points)
		p = glm::vec3(p.x - 50.0f, p.y, p.z - 100.0f);
	for (auto &[p, q] : m_letter_points)
		p = glm::vec3(p.x - 50.0f, p.y, p.z - 100.0f);
	for (auto &[p, q] : white_lid_points)
		p = glm::vec3(p.x - 50.0f, p.y, p.z - 100.0f);

	collect_parts(opaque_points, opaque_faces, button_points, button_faces);
	collect_parts(opaque_points, opaque_faces, inner_shell_points, inner_shell_faces);
	collect_parts(opaque_points, opaque_faces, lid_base_points, lid_base_faces);
	collect_parts(opaque_points, opaque_faces, bump_1_points, bump_1_faces);
	collect_parts(opaque_points, opaque_faces, bump_2_points, bump_2_faces);
	collect_parts(opaque_points, opaque_faces, m_letter_points, m_letter_faces);
	collect_parts(trans_points, trans_faces, white_lid_points, white_lid_faces);

	return *this;
}

Model &NetBall::parse()
{
	parse_parts("models/netball/button.stl", button_points, button_faces, &white_plastic);
	parse_parts("models/netball/inner_shell.stl", inner_shell_points, inner_shell_faces, &black_plastic);
	parse_parts("models/netball/lid_base.stl", lid_base_points, lid_base_faces, &cyan_plastic);
	parse_parts("models/netball/white_lid.stl", white_lid_points, white_lid_faces, &white_plastic_trans);
	parse_parts("models/netball/net.stl", net_points, net_faces, &black_plastic);

	return *this;
}

Model &NetBall::collect(Points &opaque_points, Faces &opaque_faces,
						Points &trans_points, Faces &trans_faces)
{
	for (auto &[p, q] : inner_shell_points)
		p = glm::vec3(p.x - 80.8f, p.y - 45.0f, p.z + 3.0f);

	glm::quat quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PIf32, glm::vec3(0.0f, 0.0f, 1.0f));
	for (auto &point : inner_shell_points) {
		glm::vec4 p = quat * glm::vec4(point.first, 1.0f);
		glm::vec4 q = quat * glm::vec4(point.second, 1.0f);
		point = std::make_pair(glm::vec3(p.x, p.y, p.z), glm::vec3(q.x, q.y, q.z));
	}

	for (auto &[p, q] : button_points)
		p = glm::vec3(p.x + 50.0f, p.y, p.z - 100.0f);
	for (auto &[p, q] : inner_shell_points)
		p = glm::vec3(p.x + 50.0f, p.y, p.z - 100.0f);
	for (auto &[p, q] : net_points)
		p = glm::vec3(p.x + 50.0f, p.y, p.z - 100.0f);
	for (auto &[p, q] : lid_base_points)
		p = glm::vec3(p.x + 50.0f, p.y, p.z - 100.0f);
	for (auto &[p, q] : white_lid_points)
		p = glm::vec3(p.x + 50.0f, p.y, p.z - 100.0f);

	collect_parts(opaque_points, opaque_faces, button_points, button_faces);
	collect_parts(opaque_points, opaque_faces, inner_shell_points, inner_shell_faces);
	collect_parts(opaque_points, opaque_faces, net_points, net_faces);
	collect_parts(opaque_points, opaque_faces, lid_base_points, lid_base_faces);
	collect_parts(trans_points, trans_faces, white_lid_points, white_lid_faces);

	return *this;
}

Model &TimerBall::parse()
{
	parse_parts("models/timerball/button.stl", button_points, button_faces, &white_plastic);
	parse_parts("models/timerball/inner_shell.stl", inner_shell_points, inner_shell_faces, &black_plastic);
	parse_parts("models/timerball/lid_top.stl", lid_top_points, lid_top_faces, &white_plastic);
	parse_parts("models/timerball/lid_bottom.stl", lid_bottom_points, lid_bottom_faces, &white_plastic_trans);
	parse_parts("models/timerball/lid_black.stl", lid_black_points, lid_black_faces, &black_plastic);
	parse_parts("models/timerball/stripes_lower.stl", stripes_lower_points, stripes_lower_faces, &red_plastic);
	parse_parts("models/timerball/stripes_upper.stl", stripes_upper_points, stripes_upper_faces, &red_plastic);
	parse_parts("models/timerball/top_stripe.stl", top_stripe_points, top_stripe_faces, &red_plastic);

	return *this;
}

Model &TimerBall::collect(Points &opaque_points, Faces &opaque_faces,
						  Points &trans_points, Faces &trans_faces)
{
	for (auto &[p, q] : inner_shell_points)
		p = glm::vec3(p.x - 80.8f, p.y - 45.0f, p.z + 3.0f);

	glm::quat quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PIf32, glm::vec3(0.0f, 0.0f, 1.0f));
	for (auto &point : inner_shell_points) {
		glm::vec4 p = quat * glm::vec4(point.first, 1.0f);
		glm::vec4 q = quat * glm::vec4(point.second, 1.0f);
		point = std::make_pair(glm::vec3(p.x, p.y, p.z), glm::vec3(q.x, q.y, q.z));
	}

	quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PIf32 * 1.0f / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	for (auto &point : top_stripe_points) {
		glm::vec4 p = quat * glm::vec4(point.first, 1.0f);
		glm::vec4 q = quat * glm::vec4(point.second, 1.0f);
		point = std::make_pair(glm::vec3(p.x, p.y, p.z - 1.0f), glm::vec3(q.x, q.y, q.z));
	}

	for (auto &[p, q] : button_points)
		p = glm::vec3(p.x - 100.0f, p.y, p.z);
	for (auto &[p, q] : inner_shell_points)
		p = glm::vec3(p.x - 100.0f, p.y, p.z);
	for (auto &[p, q] : lid_top_points)
		p = glm::vec3(p.x - 100.0f, p.y, p.z);
	for (auto &[p, q] : lid_bottom_points)
		p = glm::vec3(p.x - 100.0f, p.y, p.z);
	for (auto &[p, q] : lid_black_points)
		p = glm::vec3(p.x - 100.0f, p.y, p.z);
	for (auto &[p, q] : stripes_lower_points)
		p = glm::vec3(p.x - 100.0f, p.y, p.z);
	for (auto &[p, q] : stripes_upper_points)
		p = glm::vec3(p.x - 100.0f, p.y, p.z);
	for (auto &[p, q] : top_stripe_points)
		p = glm::vec3(p.x - 100.0f, p.y, p.z);

	collect_parts(opaque_points, opaque_faces, button_points, button_faces);
	collect_parts(opaque_points, opaque_faces, inner_shell_points, inner_shell_faces);
	collect_parts(opaque_points, opaque_faces, lid_top_points, lid_top_faces);
	collect_parts(trans_points, trans_faces, lid_bottom_points, lid_bottom_faces);
	collect_parts(opaque_points, opaque_faces, lid_black_points, lid_black_faces);
	collect_parts(opaque_points, opaque_faces, stripes_lower_points, stripes_lower_faces);
	collect_parts(opaque_points, opaque_faces, stripes_upper_points, stripes_upper_faces);
	collect_parts(opaque_points, opaque_faces, top_stripe_points, top_stripe_faces);

	return *this;
}

Model &UltraBall::parse()
{
	parse_parts("models/ultraball/button.stl", button_points, button_faces, &white_plastic);
	parse_parts("models/ultraball/inner_shell.stl", inner_shell_points, inner_shell_faces, &black_plastic);
	parse_parts("models/ultraball/lid_black.stl", lid_black_points, lid_black_faces, &black_plastic);
	parse_parts("models/ultraball/white_lid.stl", white_lid_points, white_lid_faces, &white_plastic_trans);
	parse_parts("models/ultraball/lid_yellow.stl", lid_yellow_points, lid_yellow_faces, &yellow_plastic);

	return *this;
}

Model &UltraBall::collect(Points &opaque_points, Faces &opaque_faces,
						  Points &trans_points, Faces &trans_faces)
{
	for (auto &[p, q] : inner_shell_points)
		p = glm::vec3(p.x - 80.8f, p.y - 45.0f, p.z + 3.0f);

	glm::quat quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PIf32, glm::vec3(0.0f, 0.0f, 1.0f));
	for (auto &point : inner_shell_points) {
		glm::vec4 p = quat * glm::vec4(point.first, 1.0f);
		glm::vec4 q = quat * glm::vec4(point.second, 1.0f);
		point = std::make_pair(glm::vec3(p.x, p.y, p.z), glm::vec3(q.x, q.y, q.z));
	}

	for (auto &[p, q] : lid_black_points)
		p = glm::vec3(p.x, p.y + 41.0f, p.z);
	for (auto &[p, q] : lid_yellow_points)
		p = glm::vec3(p.x, p.y + 41.0f, p.z);

	collect_parts(opaque_points, opaque_faces, button_points, button_faces);
	collect_parts(opaque_points, opaque_faces, inner_shell_points, inner_shell_faces);
	collect_parts(opaque_points, opaque_faces, lid_black_points, lid_black_faces);
	collect_parts(opaque_points, opaque_faces, lid_yellow_points, lid_yellow_faces);
	collect_parts(trans_points, trans_faces, white_lid_points, white_lid_faces);

	return *this;
}

Model &Table::parse()
{
	parse_parts("models/table.stl", table_points, table_faces, &gold);

	return *this;
}

Model &Table::collect(Points &opaque_points, Faces &opaque_faces,
						  Points &trans_points, Faces &trans_faces)
{
	for (auto &[p, q] : table_points)
		p = glm::vec3(p.x * 7500.0f, p.y * 7500.0f - 150.0f, p.z * 7500.0f);

	glm::quat quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PIf32 / 3.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	for (auto &point : table_points) {
		glm::vec4 p = quat * glm::vec4(point.first, 1.0f);
		glm::vec4 q = quat * glm::vec4(point.second, 1.0f);
		point = std::make_pair(glm::vec3(p.x, p.y, p.z), glm::vec3(q.x, q.y, q.z));
	}

	collect_parts(opaque_points, opaque_faces, table_points, table_faces);

	return *this;
}
