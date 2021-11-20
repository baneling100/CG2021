#ifndef MODEL_H
#define MODEL_H

#include <cstdio>
#include <cstdlib>
#include <map>
#include <vector>
#include <tuple>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "material.h"

class Model
{
  public:
	using Points = std::vector<std::pair<glm::vec3, glm::vec3>>;
	using Faces = std::vector<std::tuple<int, int, int, Material *>>;
	
	virtual Model &parse() = 0;
	virtual Model &collect(Points &opaque_points, Faces &opaque_faces,
						   Points &trans_points, Faces &trans_faces) = 0;

  protected:
	union Convert {
		float f;
		unsigned int i;
	};

	struct Comparator {
		bool operator()(const glm::vec3 &a, const glm::vec3 &b) const
		{
			return a.x < b.x || (a.x == b.x && a.y < b.y) ||
				   (a.x == b.x && a.y == b.y && a.z < b.z);
		}
	};

	static void parse_parts(std::string &&file_path, Points &points, Faces &faces,
							Material *material);
	static void collect_parts(Points &global_points, Faces &global_faces,
							  Points &local_points, Faces &local_faces);
};

class PokeBall : Model
{
  public:
	PokeBall() = default;

	Model &parse() override;
	Model &collect(Points &opaque_points, Faces &opaque_faces,
				   Points &trans_points, Faces &trans_faces) override;

  protected:
	Points button_points, inner_shell_points, red_lid_points, white_lid_points;
	Faces button_faces, inner_shell_faces, red_lid_faces, white_lid_faces;
};

class MasterBall : Model
{
  public:
	MasterBall() = default;

	Model &parse() override;
	Model &collect(Points &opaque_points, Faces &opaque_faces,
				   Points &trans_points, Faces &trans_faces) override;

  protected:
	Points button_points, inner_shell_points, lid_base_points, white_lid_points, m_letter_points,
		   bump_1_points, bump_2_points;
	Faces button_faces, inner_shell_faces, lid_base_faces, white_lid_faces, m_letter_faces,
		   bump_1_faces, bump_2_faces;
};

class NetBall : Model
{
  public:
	NetBall() = default;
	
	Model &parse() override;
	Model &collect(Points &opaque_points, Faces &opaque_faces,
				   Points &trans_points, Faces &trans_faces) override;

  protected:
	Points button_points, inner_shell_points, lid_base_points, net_points, white_lid_points;
	Faces button_faces, inner_shell_faces, lid_base_faces, net_faces, white_lid_faces;
};

class TimerBall : Model
{
  public:
	TimerBall() = default;

	Model &parse() override;
	Model &collect(Points &opaque_points, Faces &opaque_faces,
				   Points &trans_points, Faces &trans_faces) override;

  protected:
	Points button_points, inner_shell_points, lid_black_points, lid_bottom_points, lid_top_points,
		   stripes_lower_points, stripes_upper_points, top_stripe_points;
	Faces button_faces, inner_shell_faces, lid_black_faces, lid_bottom_faces, lid_top_faces,
		  stripes_lower_faces, stripes_upper_faces, top_stripe_faces;
};

class UltraBall : Model
{
  public:
	UltraBall() = default;

	Model &parse() override;
	Model &collect(Points &opaque_points, Faces &opaque_faces,
				   Points &trans_points, Faces &trans_faces) override;

  protected:
	Points button_points, inner_shell_points, lid_black_points, lid_yellow_points, white_lid_points;
	Faces button_faces, inner_shell_faces, lid_black_faces, lid_yellow_faces, white_lid_faces;
};

class Table : Model
{
  public:
	Table() = default;

	Model &parse() override;
	Model &collect(Points &opaque_points, Faces &opaque_faces,
				   Points &trans_points, Faces &trans_faces) override;

  protected:
	Points table_points;
	Faces table_faces;
};

#endif
