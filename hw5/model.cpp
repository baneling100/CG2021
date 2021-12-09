#include "model.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <unistd.h>

static void parse_stl(std::string &&file_path, Points &points, Faces &faces, Material *material, bool angulated = false)
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
		
		int idx1, idx2, idx3;
		if (angulated || point_map.find(p1) == point_map.end()) {
			idx1 = cnt++;
			point_map[p1] = idx1;
			points.emplace_back(p1, n, glm::vec2(0.0f, 0.0f));
		}
		else {
			idx1 = point_map[p1];
			auto &[p0, n0, t0] = points[idx1];
			n0 += n;
		}
		if (angulated || point_map.find(p2) == point_map.end()) {
			idx2 = cnt++;
			point_map[p2] = idx2;
			points.emplace_back(p2, n, glm::vec2(0.0f, 0.0f));
		}
		else {
			idx2 = point_map[p2];
			auto &[p0, n0, t0] = points[idx2];
			n0 += n;
		}
		if (angulated || point_map.find(p3) == point_map.end()) {
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

static void collect_part(Points &to_points, Faces &to_faces, Points &from_points, Faces &from_faces)
{
	int to_offset = to_points.size();

	for (auto &points : from_points)
		to_points.emplace_back(points);
	
	for (auto &[x, y, z, m, t] : from_faces)
		to_faces.emplace_back(std::make_tuple(x + to_offset, y + to_offset, z + to_offset, m, t));
}

static bool ray_cube_intersect(glm::vec3 &p0, glm::vec3 &u, glm::vec3 minB, glm::vec3 maxB)
{
	float tx1 = (minB.x - p0.x) / u.x;
	float tx2 = (maxB.x - p0.x) / u.x;
	float ty1 = (minB.y - p0.y) / u.y;
	float ty2 = (maxB.y - p0.y) / u.y;
	float tz1 = (minB.z - p0.z) / u.z;
	float tz2 = (maxB.z - p0.z) / u.z;

	float tmin = std::max(std::max(std::min(tx1, tx2), std::min(ty1, ty2)), std::min(tz1, tz2));
	float tmax = std::min(std::min(std::max(tx1, tx2), std::max(ty1, ty2)), std::max(tz1, tz2));

	return tmax >= 0 && tmin <= tmax;
}

static void collect_sub_indices(Points &points, Faces &faces, std::vector<int> &indices, std::vector<int> &sub_indices,
								glm::vec3 &minB, glm::vec3 &maxB)
{
	glm::vec3 midB = (minB + maxB) / 2.0f;
	float boxcenter[3] = {midB.x, midB.y, midB.z};
	glm::vec3 half = midB - minB;
	float boxhalfsize[3] = {half.x, half.y, half.z};
	for (auto index : indices) {
		auto &[i1, i2, i3, m, t] = faces[index];
		auto &[p1, n1, t1] = points[i1];
		auto &[p2, n2, t2] = points[i2];
		auto &[p3, n3, t3] = points[i3];
		float triverts[3][3] = {{p1.x, p1.y, p1.z}, {p2.x, p2.y, p2.z}, {p3.x, p3.y, p3.z}};
		if (triBoxOverlap(boxcenter, boxhalfsize, triverts))
			sub_indices.push_back(index);
	}
}

static void make_octree(OctreeNode *now, Points &points, Faces &faces, std::vector<int> &indices, int depth)
{
	if (indices.size() <= 12 || depth > 6) {
		now->size = indices.size();
		now->indices = (int *)malloc(now->size * sizeof(int));
		for (int i = 0; i < now->size; i++)
			now->indices[i] = indices[i];
	}
	else {
		now->size = 0;
		glm::vec3 midB = (now->minB + now->maxB) / 2.0f;
		float rangeX[8][2] = {{now->minB.x, midB.x}, {now->minB.x, midB.x}, {now->minB.x, midB.x}, {now->minB.x, midB.x},
							  {midB.x, now->maxB.x}, {midB.x, now->maxB.x}, {midB.x, now->maxB.x}, {midB.x, now->maxB.x}};
		float rangeY[8][2] = {{now->minB.y, midB.y}, {now->minB.y, midB.y}, {midB.y, now->maxB.y}, {midB.y, now->maxB.y},
							  {now->minB.y, midB.y}, {now->minB.y, midB.y}, {midB.y, now->maxB.y}, {midB.y, now->maxB.y}};
		float rangeZ[8][2] = {{now->minB.z, midB.z}, {midB.z, now->maxB.z}, {now->minB.z, midB.z}, {midB.z, now->maxB.z},
							  {now->minB.z, midB.z}, {midB.z, now->maxB.z}, {now->minB.z, midB.z}, {midB.z, now->maxB.z}};
		std::vector<int> sub_indices;
		for (int i = 0; i < 8; i++) {
			sub_indices.clear();
			glm::vec3 minB = glm::vec3(rangeX[i][0], rangeY[i][0], rangeZ[i][0]);
			glm::vec3 maxB = glm::vec3(rangeX[i][1], rangeY[i][1], rangeZ[i][1]);
			collect_sub_indices(points, faces, indices, sub_indices, minB, maxB);
			if (sub_indices.size()) {
				now->nodes[i] = (OctreeNode *)malloc(sizeof(OctreeNode));
				now->nodes[i]->minB = minB;
				now->nodes[i]->maxB = maxB;
				make_octree(now->nodes[i], points, faces, sub_indices, depth + 1);
			}
			else now->nodes[i] = NULL;
		}
	}
}

void Model::collect(Points &global_points, Faces &opaque_faces, Faces &trans_faces, Images &images)
{
	offset = global_points.size();
	
	for (auto &points : local_points)
		global_points.emplace_back(points);
	
	for (auto &[x, y, z, m, t] : local_faces) {
		if (m && (m->ambient[3] < 1.0f || m->diffuse[3] < 1.0f || m->specular[3] < 1.0f))
			trans_faces.emplace_back(std::make_tuple(x + offset, y + offset, z + offset, m, t));
		else opaque_faces.emplace_back(std::make_tuple(x + offset, y + offset, z + offset, m, t));
	}

	if (images.size() < texture)
		for (unsigned int i = images.size(); i < texture; i++)
			images.emplace_back(0, 0, nullptr);
	images.emplace_back(width, height, image);
}

static std::tuple<float, Point, Material *, unsigned int, bool>
recursive_nearest_intersect(Points &points, Faces &faces, glm::vec3 &p0, glm::vec3 &u, OctreeNode *now, glm::vec3 &motion)
{
	float min_dist = 999999999.0f;
	Point p = std::make_tuple(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f));
	Material *mat = nullptr; unsigned int tex = 0;
	bool is_valid = false;

	if (now->size) {
		for (int i = 0; i < now->size; i++) {
			auto &[p1, p2, p3, mi, ti] = faces[now->indices[i]];
			auto &[v1, n1, t1] = points[p1];
			auto &[v2, n2, t2] = points[p2];
			auto &[v3, n3, t3] = points[p3];
			glm::vec3 tv1 = v1 + motion, tv2 = v2 + motion, tv3 = v3 + motion;
			glm::vec2 bary; float dist;
			if (glm::intersectRayTriangle(p0, u, tv1, tv2, tv3, bary, dist) && dist >= 0.001f && min_dist > dist) {
				is_valid = true;
				min_dist = dist;
				glm::vec3 v = (1.0f - bary.x - bary.y) * tv1 + bary.x * tv2 + bary.y * tv3;
				glm::vec3 n = (1.0f - bary.x - bary.y) * n1 + bary.x * n2 + bary.y * n3;
				glm::vec2 t = (1.0f - bary.x - bary.y) * t1 + bary.x * t2 + bary.y * t3;
				p = std::make_tuple(v, n, t);
				mat = mi;
				tex = ti;
			}
		}
		return std::make_tuple(min_dist, p, mat, tex, is_valid);
	}

	for (int i = 0; i < 8; i++)
		if (now->nodes[i] && ray_cube_intersect(p0, u, now->nodes[i]->minB + motion, now->nodes[i]->maxB + motion)) {
			auto [min_dist_i, p_i, mat_i, tex_i, is_valid_i] = recursive_nearest_intersect(points, faces, p0, u, now->nodes[i], motion);
			if (is_valid_i && min_dist > min_dist_i) {
				is_valid = true;
				min_dist = min_dist_i;
				p = p_i;
				mat = mat_i;
				tex = tex_i;
			}
		}
	return std::make_tuple(min_dist, p, mat, tex, is_valid);
}

std::tuple<float, Point, Material *, unsigned int, bool> Model::nearest_intersect(glm::vec3 &p0, glm::vec3 &u, Option option)
{
	if (ray_cube_intersect(p0, u, top->minB + motion, top->maxB + motion))
		return recursive_nearest_intersect(local_points, local_faces, p0, u, top, motion);
	return std::make_tuple(999999999.0f, std::make_tuple(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)), nullptr, 0, false);
}

static float recursive_shadow_attenuation(Points &points, Faces &faces,
									glm::vec3 &p0, glm::vec3 &u, OctreeNode *now, glm::vec3 &motion)
{
	float SA = 1.0f;

	if (now->size) {
		for (int i = 0; i < now->size; i++) {
			auto &[p1, p2, p3, mi, ti] = faces[now->indices[i]];
			auto &[v1, n1, t1] = points[p1];
			auto &[v2, n2, t2] = points[p2];
			auto &[v3, n3, t3] = points[p3];
			glm::vec3 tv1 = v1 + motion, tv2 = v2 + motion, tv3 = v3 + motion;
			glm::vec2 bary; float dist;
			if (glm::intersectRayTriangle(p0, u, tv1, tv2, tv3, bary, dist) && dist >= 0.001f) {
				SA *= 1.0f - mi->ambient[3];
				if (SA == 0.0f) return 0.0f;
			}
		}
		return SA;
	}

	for (int i = 0; i < 8; i++)
		if (now->nodes[i] && ray_cube_intersect(p0, u, now->nodes[i]->minB + motion, now->nodes[i]->maxB + motion)) {
			SA *= recursive_shadow_attenuation(points, faces, p0, u, now->nodes[i], motion);
			if (SA == 0.0f) return 0.0f;
		}
	return SA;
}

float Model::shadow_attenuation(glm::vec3 &p0, glm::vec3 &u)
{
	if (ray_cube_intersect(p0, u, top->minB + motion, top->maxB + motion))
		return recursive_shadow_attenuation(local_points, local_faces, p0, u, top, motion);
	return 1.0f;
}

void Model::set_motion(glm::vec3 m)
{
	motion = m;
}

Model &PokeBall::parse()
{
	local_points.clear(); local_faces.clear();

	Points button_points, inner_shell_points, red_lid_points, white_lid_points;
	Faces button_faces, inner_shell_faces, red_lid_faces, white_lid_faces;

	parse_stl("models/pokeball/button.stl", button_points, button_faces, &glass);
	parse_stl("models/pokeball/inner_shell.stl", inner_shell_points, inner_shell_faces, &black_plastic);
	parse_stl("models/pokeball/red_lid.stl", red_lid_points, red_lid_faces, &red_plastic);
	parse_stl("models/pokeball/white_lid.stl", white_lid_points, white_lid_faces, &glass);

	for (auto &[p, n, t] : inner_shell_points)
		p += glm::vec3(-80.7f, -45.1f, 3.0f);

	glm::quat quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PIf32, glm::vec3(0.0f, 0.0f, 1.0f));
	for (auto &[p, n, t] : inner_shell_points) { p = quat * p; n = quat * n; }

	for (auto &[p, n, t] : button_points)      p += glm::vec3(-130.9f, 0.0f, -95.1f);
	for (auto &[p, n, t] : inner_shell_points) p += glm::vec3(-130.9f, 0.0f, -95.1f);
	for (auto &[p, n, t] : red_lid_points)     p += glm::vec3(-130.9f, 0.0f, -95.1f);
	for (auto &[p, n, t] : white_lid_points)   p += glm::vec3(-130.9f, 0.0f, -95.1f);
	
	collect_part(local_points, local_faces, button_points, button_faces);
	collect_part(local_points, local_faces, inner_shell_points, inner_shell_faces);
	collect_part(local_points, local_faces, red_lid_points, red_lid_faces);
	collect_part(local_points, local_faces, white_lid_points, white_lid_faces);
	
	for (auto &[p, n, t] : local_points) p /= 100.0f;

	top = (OctreeNode *)malloc(sizeof(OctreeNode));
	top->minB = glm::vec3(999999999.0f, 999999999.0f, 999999999.0f);
	top->maxB = glm::vec3(-999999999.0f, -999999999.0f, -999999999.0f);
	for (auto &[p, n, t] : local_points) {
		top->minB.x = std::min(top->minB.x, p.x); top->maxB.x = std::max(top->maxB.x, p.x);
		top->minB.y = std::min(top->minB.y, p.y); top->maxB.y = std::max(top->maxB.y, p.y);
		top->minB.z = std::min(top->minB.z, p.z); top->maxB.z = std::max(top->maxB.z, p.z);
	}
	std::vector<int> indices;
	int size = local_faces.size();
	for (int i = 0; i < size; i++) indices.push_back(i);
	make_octree(top, local_points, local_faces, indices, 0);

	return *this;
}

Model &MasterBall::parse()
{
	local_points.clear(); local_faces.clear();

	Points button_points, inner_shell_points, lid_base_points, white_lid_points, m_letter_points,
		   bump_1_points, bump_2_points;
	Faces button_faces, inner_shell_faces, lid_base_faces, white_lid_faces, m_letter_faces,
		  bump_1_faces, bump_2_faces;
		  
	parse_stl("models/masterball/button.stl", button_points, button_faces, &glass);
	parse_stl("models/masterball/inner_shell.stl", inner_shell_points, inner_shell_faces, &black_plastic);
	parse_stl("models/masterball/lid_base.stl", lid_base_points, lid_base_faces, &purple_plastic);
	parse_stl("models/masterball/white_lid.stl", white_lid_points, white_lid_faces, &glass);
	parse_stl("models/masterball/m_letter.stl", m_letter_points, m_letter_faces, &white_plastic);
	parse_stl("models/masterball/bump_1.stl", bump_1_points, bump_1_faces, &pink_plastic);
	parse_stl("models/masterball/bump_2.stl", bump_2_points, bump_2_faces, &pink_plastic);

	for (auto &[p, n, t] : inner_shell_points)
		p += glm::vec3(-80.7f, -45.1f, 3.0f);

	glm::quat quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PIf32, glm::vec3(0.0f, 0.0f, 1.0f));
	for (auto &[p, n, t] : inner_shell_points) { p = quat * p; n = quat * n; }

	for (auto &[p, n, t] : lid_base_points) p += glm::vec3(-12.5f, 41.0f, 0.0f);
	for (auto &[p, n, t] : bump_1_points)   p += glm::vec3(-12.5f, 41.0f, 0.0f);
	for (auto &[p, n, t] : bump_2_points)   p += glm::vec3(-12.5f, 41.0f, 0.0f);
	for (auto &[p, n, t] : m_letter_points) p += glm::vec3(-12.5f, 41.0f, 0.0f);
	for (auto &[p, n, t] : button_points)      p += glm::vec3(-50.0f, 0.0f, -153.9f);
	for (auto &[p, n, t] : inner_shell_points) p += glm::vec3(-50.0f, 0.0f, -153.9f);
	for (auto &[p, n, t] : lid_base_points)    p += glm::vec3(-50.0f, 0.0f, -153.9f);
	for (auto &[p, n, t] : bump_1_points)      p += glm::vec3(-50.0f, 0.0f, -153.9f);
	for (auto &[p, n, t] : bump_2_points)      p += glm::vec3(-50.0f, 0.0f, -153.9f);
	for (auto &[p, n, t] : m_letter_points)    p += glm::vec3(-50.0f, 0.0f, -153.9f);
	for (auto &[p, n, t] : white_lid_points)   p += glm::vec3(-50.0f, 0.0f, -153.9f);

	collect_part(local_points, local_faces, button_points, button_faces);
	collect_part(local_points, local_faces, inner_shell_points, inner_shell_faces);
	collect_part(local_points, local_faces, lid_base_points, lid_base_faces);
	collect_part(local_points, local_faces, bump_1_points, bump_1_faces);
	collect_part(local_points, local_faces, bump_2_points, bump_2_faces);
	collect_part(local_points, local_faces, m_letter_points, m_letter_faces);
	collect_part(local_points, local_faces, white_lid_points, white_lid_faces);

	for (auto &[p, n, t] : local_points) p /= 100.0f;

	top = (OctreeNode *)malloc(sizeof(OctreeNode));
	top->minB = glm::vec3(999999999.0f, 999999999.0f, 999999999.0f);
	top->maxB = glm::vec3(-999999999.0f, -999999999.0f, -999999999.0f);
	for (auto &[p, n, t] : local_points) {
		top->minB.x = std::min(top->minB.x, p.x); top->maxB.x = std::max(top->maxB.x, p.x);
		top->minB.y = std::min(top->minB.y, p.y); top->maxB.y = std::max(top->maxB.y, p.y);
		top->minB.z = std::min(top->minB.z, p.z); top->maxB.z = std::max(top->maxB.z, p.z);
	}
	std::vector<int> indices;
	int size = local_faces.size();
	for (int i = 0; i < size; i++) indices.push_back(i);
	make_octree(top, local_points, local_faces, indices, 0);

	return *this;
}

Model &NetBall::parse()
{
	local_points.clear(); local_faces.clear();

	Points button_points, inner_shell_points, lid_base_points, net_points, white_lid_points;
	Faces button_faces, inner_shell_faces, lid_base_faces, net_faces, white_lid_faces;

	parse_stl("models/netball/button.stl", button_points, button_faces, &glass);
	parse_stl("models/netball/inner_shell.stl", inner_shell_points, inner_shell_faces, &black_plastic);
	parse_stl("models/netball/lid_base.stl", lid_base_points, lid_base_faces, &cyan_plastic);
	parse_stl("models/netball/white_lid.stl", white_lid_points, white_lid_faces, &glass);
	parse_stl("models/netball/net.stl", net_points, net_faces, &black_plastic);

	for (auto &[p, n, t] : inner_shell_points)
		p += glm::vec3(-80.7f, -45.1f, 3.0f);

	glm::quat quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PIf32, glm::vec3(0.0f, 0.0f, 1.0f));
	for (auto &[p, n, t] : inner_shell_points) { p = quat * p; n = quat * n; }

	for (auto &[p, n, t] : button_points)      p += glm::vec3(30.9f, 0.0f, -95.1f);
	for (auto &[p, n, t] : inner_shell_points) p += glm::vec3(30.9f, 0.0f, -95.1f);
	for (auto &[p, n, t] : net_points)         p += glm::vec3(30.9f, 0.0f, -98.3f);
	for (auto &[p, n, t] : lid_base_points)    p += glm::vec3(30.9f, 0.0f, -98.3f);
	for (auto &[p, n, t] : white_lid_points)   p += glm::vec3(30.9f, 0.0f, -95.1f);

	collect_part(local_points, local_faces, button_points, button_faces);
	collect_part(local_points, local_faces, inner_shell_points, inner_shell_faces);
	collect_part(local_points, local_faces, net_points, net_faces);
	collect_part(local_points, local_faces, lid_base_points, lid_base_faces);
	collect_part(local_points, local_faces, white_lid_points, white_lid_faces);

	for (auto &[p, n, t] : local_points) p /= 100.0f;

	top = (OctreeNode *)malloc(sizeof(OctreeNode));
	top->minB = glm::vec3(999999999.0f, 999999999.0f, 999999999.0f);
	top->maxB = glm::vec3(-999999999.0f, -999999999.0f, -999999999.0f);
	for (auto &[p, n, t] : local_points) {
		top->minB.x = std::min(top->minB.x, p.x); top->maxB.x = std::max(top->maxB.x, p.x);
		top->minB.y = std::min(top->minB.y, p.y); top->maxB.y = std::max(top->maxB.y, p.y);
		top->minB.z = std::min(top->minB.z, p.z); top->maxB.z = std::max(top->maxB.z, p.z);
	}
	std::vector<int> indices;
	int size = local_faces.size();
	for (int i = 0; i < size; i++) indices.push_back(i);
	make_octree(top, local_points, local_faces, indices, 0);

	return *this;
}

Model &TimerBall::parse()
{
	local_points.clear(); local_faces.clear();

	Points button_points, inner_shell_points, lid_black_points, lid_bottom_points, lid_top_points,
		   stripes_lower_points, stripes_upper_points, top_stripe_points;
	Faces button_faces, inner_shell_faces, lid_black_faces, lid_bottom_faces, lid_top_faces,
		  stripes_lower_faces, stripes_upper_faces, top_stripe_faces;

	parse_stl("models/timerball/button.stl", button_points, button_faces, &glass);
	parse_stl("models/timerball/inner_shell.stl", inner_shell_points, inner_shell_faces, &black_plastic);
	parse_stl("models/timerball/lid_top.stl", lid_top_points, lid_top_faces, &white_plastic);
	parse_stl("models/timerball/lid_bottom.stl", lid_bottom_points, lid_bottom_faces, &glass);
	parse_stl("models/timerball/lid_black.stl", lid_black_points, lid_black_faces, &black_plastic);
	parse_stl("models/timerball/stripes_lower.stl", stripes_lower_points, stripes_lower_faces, &red_plastic);
	parse_stl("models/timerball/stripes_upper.stl", stripes_upper_points, stripes_upper_faces, &red_plastic);
	parse_stl("models/timerball/top_stripe.stl", top_stripe_points, top_stripe_faces, &red_plastic);

	for (auto &[p, n, t] : inner_shell_points) p += glm::vec3(-80.7f, -45.1f, 3.0f);
	glm::quat quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PIf32, glm::vec3(0.0f, 0.0f, 1.0f));
	for (auto &[p, n, t] : inner_shell_points) { p = quat * p; n = quat * n; }
	for (auto &[p, n, t] : button_points)        p.x -= 100.0f;
	for (auto &[p, n, t] : inner_shell_points)   p.x -= 100.0f;
	for (auto &[p, n, t] : lid_top_points)       p.x -= 100.0f;
	for (auto &[p, n, t] : lid_bottom_points)    p.x -= 100.0f;
	for (auto &[p, n, t] : lid_black_points)     p.x -= 100.0f;
	for (auto &[p, n, t] : stripes_lower_points) p.x -= 100.0f;
	for (auto &[p, n, t] : stripes_upper_points) p.x -= 100.0f;
	for (auto &[p, n, t] : top_stripe_points)    p.x -= 100.0f;

	collect_part(local_points, local_faces, button_points, button_faces);
	collect_part(local_points, local_faces, inner_shell_points, inner_shell_faces);
	collect_part(local_points, local_faces, lid_top_points, lid_top_faces);
	collect_part(local_points, local_faces, lid_bottom_points, lid_bottom_faces);
	collect_part(local_points, local_faces, lid_black_points, lid_black_faces);
	collect_part(local_points, local_faces, stripes_lower_points, stripes_lower_faces);
	collect_part(local_points, local_faces, stripes_upper_points, stripes_upper_faces);
	collect_part(local_points, local_faces, top_stripe_points, top_stripe_faces);

	for (auto &[p, n, t] : local_points) p /= 100.0f;

	top = (OctreeNode *)malloc(sizeof(OctreeNode));
	top->minB = glm::vec3(999999999.0f, 999999999.0f, 999999999.0f);
	top->maxB = glm::vec3(-999999999.0f, -999999999.0f, -999999999.0f);
	for (auto &[p, n, t] : local_points) {
		top->minB.x = std::min(top->minB.x, p.x); top->maxB.x = std::max(top->maxB.x, p.x);
		top->minB.y = std::min(top->minB.y, p.y); top->maxB.y = std::max(top->maxB.y, p.y);
		top->minB.z = std::min(top->minB.z, p.z); top->maxB.z = std::max(top->maxB.z, p.z);
	}
	std::vector<int> indices;
	int size = local_faces.size();
	for (int i = 0; i < size; i++) indices.push_back(i);
	make_octree(top, local_points, local_faces, indices, 0);

	return *this;
}

Model &UltraBall::parse()
{
	local_points.clear(); local_faces.clear();

	Points button_points, inner_shell_points, lid_black_points, lid_yellow_points, white_lid_points;
	Faces button_faces, inner_shell_faces, lid_black_faces, lid_yellow_faces, white_lid_faces;

	parse_stl("models/ultraball/button.stl", button_points, button_faces, &glass);
	parse_stl("models/ultraball/inner_shell.stl", inner_shell_points, inner_shell_faces, &black_plastic);
	parse_stl("models/ultraball/lid_black.stl", lid_black_points, lid_black_faces, &black_plastic);
	parse_stl("models/ultraball/white_lid.stl", white_lid_points, white_lid_faces, &glass);
	parse_stl("models/ultraball/lid_yellow.stl", lid_yellow_points, lid_yellow_faces, &yellow_plastic);

	for (auto &[p, n, t] : inner_shell_points) p += glm::vec3(-80.7f, -45.1f, 3.0f);

	glm::quat quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PIf32, glm::vec3(0.0f, 0.0f, 1.0f));
	for (auto &[p, n, t] : inner_shell_points) { p = quat * p; n = quat * n; }

	for (auto &[p, n, t] : lid_black_points)   p.y += 41.0f;
	for (auto &[p, n, t] : lid_yellow_points)  p.y += 41.0f;

	collect_part(local_points, local_faces, button_points, button_faces);
	collect_part(local_points, local_faces, inner_shell_points, inner_shell_faces);
	collect_part(local_points, local_faces, lid_black_points, lid_black_faces);
	collect_part(local_points, local_faces, lid_yellow_points, lid_yellow_faces);
	collect_part(local_points, local_faces, white_lid_points, white_lid_faces);

	for (auto &[p, n, t] : local_points) p /= 100.0f;

	top = (OctreeNode *)malloc(sizeof(OctreeNode));
	top->minB = glm::vec3(999999999.0f, 999999999.0f, 999999999.0f);
	top->maxB = glm::vec3(-999999999.0f, -999999999.0f, -999999999.0f);
	for (auto &[p, n, t] : local_points) {
		top->minB.x = std::min(top->minB.x, p.x); top->maxB.x = std::max(top->maxB.x, p.x);
		top->minB.y = std::min(top->minB.y, p.y); top->maxB.y = std::max(top->maxB.y, p.y);
		top->minB.z = std::min(top->minB.z, p.z); top->maxB.z = std::max(top->maxB.z, p.z);
	}
	std::vector<int> indices;
	int size = local_faces.size();
	for (int i = 0; i < size; i++) indices.push_back(i);
	make_octree(top, local_points, local_faces, indices, 0);

	return *this;
}

Model &Table::parse()
{
	local_points.clear(); local_faces.clear();

	parse_stl("models/table.stl", local_points, local_faces, &green_plastic, true);

	for (auto &[p, n, t] : local_points) p = 7500.0f * p + glm::vec3(-50.0f, -250.0f, 0.0f);

	for (auto &[p, n, t] : local_points) p /= 100.0f;

	top = (OctreeNode *)malloc(sizeof(OctreeNode));
	top->minB = glm::vec3(999999999.0f, 999999999.0f, 999999999.0f);
	top->maxB = glm::vec3(-999999999.0f, -999999999.0f, -999999999.0f);
	for (auto &[p, n, t] : local_points) {
		top->minB.x = std::min(top->minB.x, p.x); top->maxB.x = std::max(top->maxB.x, p.x);
		top->minB.y = std::min(top->minB.y, p.y); top->maxB.y = std::max(top->maxB.y, p.y);
		top->minB.z = std::min(top->minB.z, p.z); top->maxB.z = std::max(top->maxB.z, p.z);
	}
	std::vector<int> indices;
	int size = local_faces.size();
	for (int i = 0; i < size; i++) indices.push_back(i);
	make_octree(top, local_points, local_faces, indices, 0);

	return *this;
}

Model &MirrorSphere::parse()
{
	local_points.clear(); local_faces.clear();
	
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
				local_points.emplace_back(p1, p1, glm::vec2(0.0f, 0.0f));
			}
			else idx1 = point_map[p1];
			if (point_map.find(p2) == point_map.end()) {
				idx2 = cnt++;
				point_map[p2] = idx2;
				local_points.emplace_back(p2, p2, glm::vec2(0.0f, 0.0f));
			}
			else idx2 = point_map[p2];
			if (point_map.find(p3) == point_map.end()) {
				idx3 = cnt++;
				point_map[p3] = idx3;
				local_points.emplace_back(p3, p3, glm::vec2(0.0f, 0.0f));
			}
			else idx3 = point_map[p3];
			if (point_map.find(p4) == point_map.end()) {
				idx3 = cnt++;
				point_map[p4] = idx4;
				local_points.emplace_back(p4, p4, glm::vec2(0.0f, 0.0f));
			}
			else idx4 = point_map[p4];

			local_faces.emplace_back(idx1, idx2, idx3, &mirror, 0);
			local_faces.emplace_back(idx3, idx4, idx1, &mirror, 0);
		}

	for (auto &[p, n, t] : local_points) p = 40.0f * p + glm::vec3(-7.0f, -51.0f, 85.4f);
	for (auto &[p, n, t] : local_points) p /= 100.0f;

	return *this;
}

std::tuple<float, Point, Material *, unsigned int, bool> MirrorSphere::nearest_intersect(glm::vec3 &p0, glm::vec3 &u, Option option)
{
	glm::vec3 p, n;

	if (glm::intersectRaySphere(p0, u, glm::vec3(-0.07f, -0.51f, 0.854f) + motion, 0.4f, p, n) && glm::distance(p0, p) >= 0.001f) {
		n = glm::normalize(n);
		float cos_theta = n.y;
		float theta = std::acos(cos_theta);
		float sin_theta = std::sqrt(1 - cos_theta * cos_theta);
		float cos_phi = n.x / sin_theta, sin_phi = n.z / sin_theta;
		float phi = std::atan2(sin_phi, cos_phi);
		if (phi < 0) phi += 2.0f * M_PIf32;
		if (option == BUMP_MAPPING) {
			static float a = 0.025f, b = 16.0f;
			float r = 0.4f + a * std::cos(b * phi) * std::sin(b * theta);
			float drdphi = -a * b * std::sin(b * phi) * std::sin(b * theta);
			float drdtheta = a * b * std::cos(b * phi) * std::cos(b * theta);
			glm::vec3 dndphi = glm::vec3(-sin_theta * sin_phi, 0.0f, sin_theta * cos_phi);
			glm::vec3 dndtheta = glm::vec3(cos_theta * cos_phi, -sin_theta, cos_theta * sin_phi);
			glm::vec3 dpdphi = drdphi * n + r * dndphi;
			glm::vec3 dpdtheta = drdtheta * n + r * dndtheta;
			n = glm::normalize(n + drdphi * glm::cross(n, dpdphi) + drdtheta * glm::cross(n, dpdtheta));
		}
		return std::make_tuple(glm::distance(p0, p), std::make_tuple(p, n, glm::vec2(0.0f, 0.0f)), &mirror, 0, true);
	}
	return std::make_tuple(999999999.0f, std::make_tuple(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)), nullptr, 0, false);
}

float MirrorSphere::shadow_attenuation(glm::vec3 &p0, glm::vec3 &u)
{
	float dist;

	if (glm::intersectRaySphere(p0, u, glm::vec3(-0.07f, -0.51f, 0.854f) + motion, 0.16f, dist) && dist >= 0.001f)
		return 0.0f;
	return 1.0f;
}

Model &WoodenSphere::parse()
{
	local_points.clear(); local_faces.clear();

	int num_channels;
	image = stbi_load("textures/wood.jpg", &width, &height, &num_channels, 3);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	int h = 32;
	int w = h * width / height;
	int cnt = 0;
	std::map<glm::vec3, int, PointCompare> point_map;

	for (int i = 1; i < h; i++)
		for (int j = 1; j < w; j++) {
			float theta0 = M_PIf32 * (i - 1) / (h - 1);
			float theta1 = M_PIf32 * i / (h - 1);
			float phi0 = 2.0f * M_PIf32 * (j - 1) / (w - 1);
			float phi1 = 2.0f * M_PIf32 * j / (w - 1);

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
				local_points.emplace_back(p1, p1, t1);
			}
			else idx1 = point_map[p1];
			if (point_map.find(p2) == point_map.end()) {
				idx2 = cnt++;
				point_map[p2] = idx2;
				local_points.emplace_back(p2, p2, t2);
			}
			else idx2 = point_map[p2];
			if (point_map.find(p3) == point_map.end()) {
				idx3 = cnt++;
				point_map[p3] = idx3;
				local_points.emplace_back(p3, p3, t3);
			}
			else idx3 = point_map[p3];
			if (point_map.find(p4) == point_map.end()) {
				idx3 = cnt++;
				point_map[p4] = idx4;
				local_points.emplace_back(p4, p4, t4);
			}
			else idx4 = point_map[p4];

			local_faces.emplace_back(idx1, idx2, idx3, &default_material, texture);
			local_faces.emplace_back(idx3, idx4, idx1, &default_material, texture);
		}

	for (auto &[p, n, t] : local_points) p = 40.0f * p + glm::vec3(93.7f, -51.0f, -87.7f);
	for (auto &[p, n, t] : local_points) p /= 100.0f;

	return *this;
}

std::tuple<float, Point, Material *, unsigned int, bool> WoodenSphere::nearest_intersect(glm::vec3 &p0, glm::vec3 &u, Option option)
{
	glm::vec3 p, n;

	if (glm::intersectRaySphere(p0, u, glm::vec3(0.937f, -0.51f, -0.877f) + motion, 0.4f, p, n) && glm::distance(p0, p) >= 0.001f) {
		n = glm::normalize(n);
		float cos_theta = n.y;
		float theta = std::acos(cos_theta);
		float sin_theta = std::sqrt(1 - cos_theta * cos_theta);
		float cos_phi = n.x / sin_theta, sin_phi = n.z / sin_theta;
		float phi = std::atan2(sin_phi, cos_phi);
		if (phi < 0) phi += 2.0f * M_PIf32;
		return std::make_tuple(glm::distance(p0, p), std::make_tuple(p, n, glm::vec2(phi / (2.0f * M_PIf32), theta / M_PIf32)), &default_material, texture, true);
	}
	return std::make_tuple(999999999.0f, std::make_tuple(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)), nullptr, 0, false);
}

float WoodenSphere::shadow_attenuation(glm::vec3 &p0, glm::vec3 &u)
{
	float dist;

	if (glm::intersectRaySphere(p0, u, glm::vec3(0.937f, -0.51f, -0.877f) + motion, 0.16f, dist) && dist >= 0.001f)
		return 0.0f;
	return 1.0f;
}

Model &Dice::parse()
{
	local_points.clear(); local_faces.clear();

	int num_channels;
	image = stbi_load("textures/dice.png", &width, &height, &num_channels, 3);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	local_points.emplace_back(glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.00f, 0.50f));
	local_points.emplace_back(glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.00f, 0.75f));
	local_points.emplace_back(glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.25f, 0.75f));
	local_points.emplace_back(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.25f, 0.50f));
	local_points.emplace_back(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.25f, 0.50f));
	local_points.emplace_back(glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.25f, 0.75f));
	local_points.emplace_back(glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.50f, 0.75f));
	local_points.emplace_back(glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.50f, 0.50f));
	local_points.emplace_back(glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.50f, 0.50f));
	local_points.emplace_back(glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.50f, 0.75f));
	local_points.emplace_back(glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.75f, 0.75f));
	local_points.emplace_back(glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.75f, 0.50f));
	local_points.emplace_back(glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.75f, 0.50f));
	local_points.emplace_back(glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.75f, 0.75f));
	local_points.emplace_back(glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.00f, 0.75f));
	local_points.emplace_back(glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.00f, 0.50f));
	local_points.emplace_back(glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.50f, 0.75f));
	local_points.emplace_back(glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.50f, 1.00f));
	local_points.emplace_back(glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.75f, 1.00f));
	local_points.emplace_back(glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.75f, 0.75f));
	local_points.emplace_back(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.50f, 0.25f));
	local_points.emplace_back(glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.50f, 0.50f));
	local_points.emplace_back(glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.75f, 0.50f));
	local_points.emplace_back(glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.75f, 0.25f));

	local_faces.emplace_back( 0,  1,  2, &default_material, texture);
	local_faces.emplace_back( 2,  3,  0, &default_material, texture);
	local_faces.emplace_back( 4,  5,  6, &default_material, texture);
	local_faces.emplace_back( 6,  7,  4, &default_material, texture);
	local_faces.emplace_back( 8,  9, 10, &default_material, texture);
	local_faces.emplace_back(10, 11,  8, &default_material, texture);
	local_faces.emplace_back(12, 13, 14, &default_material, texture);
	local_faces.emplace_back(14, 15, 12, &default_material, texture);
	local_faces.emplace_back(16, 17, 18, &default_material, texture);
	local_faces.emplace_back(18, 19, 16, &default_material, texture);
	local_faces.emplace_back(20, 21, 22, &default_material, texture);
	local_faces.emplace_back(22, 23, 20, &default_material, texture);

	glm::quat quat = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), M_PI_4f32, glm::vec3(1.0f, 0.0f, 1.0f));
	for (auto &[p, n, t] : local_points) {
		p = 30.0f * (quat * p) + glm::vec3(-107.0f, -39.0f, -87.7f);
		n = quat * n;
	}

	for (auto &[p, n, t] : local_points) p /= 100.0f;

	top = (OctreeNode *)malloc(sizeof(OctreeNode));
	top->minB = glm::vec3(999999999.0f, 999999999.0f, 999999999.0f);
	top->maxB = glm::vec3(-999999999.0f, -999999999.0f, -999999999.0f);
	for (auto &[p, n, t] : local_points) {
		top->minB.x = std::min(top->minB.x, p.x); top->maxB.x = std::max(top->maxB.x, p.x);
		top->minB.y = std::min(top->minB.y, p.y); top->maxB.y = std::max(top->maxB.y, p.y);
		top->minB.z = std::min(top->minB.z, p.z); top->maxB.z = std::max(top->maxB.z, p.z);
	}
	std::vector<int> indices;
	int size = local_faces.size();
	for (int i = 0; i < size; i++) indices.push_back(i);
	make_octree(top, local_points, local_faces, indices, 0);

	return *this;
}

Model &Background::parse()
{
	local_points.clear(); local_faces.clear();

	int  num_channels;
	image = stbi_load("textures/background.jpg", &width, &height, &num_channels, 3);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

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
				local_points.emplace_back(p1, p1, t1);
			}
			else idx1 = point_map[p1];
			if (point_map.find(p2) == point_map.end()) {
				idx2 = cnt++;
				point_map[p2] = idx2;
				local_points.emplace_back(p2, p2, t2);
			}
			else idx2 = point_map[p2];
			if (point_map.find(p3) == point_map.end()) {
				idx3 = cnt++;
				point_map[p3] = idx3;
				local_points.emplace_back(p3, p3, t3);
			}
			else idx3 = point_map[p3];
			if (point_map.find(p4) == point_map.end()) {
				idx3 = cnt++;
				point_map[p4] = idx4;
				local_points.emplace_back(p4, p4, t4);
			}
			else idx4 = point_map[p4];

			local_faces.emplace_back(idx1, idx2, idx3, nullptr, texture);
			local_faces.emplace_back(idx3, idx4, idx1, nullptr, texture);
		}

	for (auto &[p, n, t] : local_points) p = 1250.0f * p;
	for (auto &[p, n, t] : local_points) p /= 100.0f;

	return *this;
}

std::tuple<float, Point, Material *, unsigned int, bool> Background::nearest_intersect(glm::vec3 &p0, glm::vec3 &u, Option option)
{
	glm::vec3 p, n;

	if (glm::intersectRaySphere(p0, u, glm::vec3(0.0f, 0.0f, 0.0f), 12.5f, p, n) && glm::distance(p0, p) >= 0.001f) {
		glm::vec3 q = glm::normalize(p);
		float cos_theta = q.y;
		float theta = std::acos(cos_theta);
		float sin_theta = std::sqrt(1 - cos_theta * cos_theta);
		float cos_phi = q.x / sin_theta, sin_phi = q.z / sin_theta;
		float phi = std::atan2(sin_phi, cos_phi);
		if (phi < -M_PI_4f32) phi += 2.0f * M_PIf32;
		phi += M_PI_4f32;
		return std::make_tuple(glm::distance(p0, p), std::make_tuple(p, n, glm::vec2(phi / (2.0f * M_PIf32), theta / M_PIf32)), nullptr, texture, true);
	}
	return std::make_tuple(999999999.0f, std::make_tuple(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)), nullptr, 0, false);
}
