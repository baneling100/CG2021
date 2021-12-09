#ifndef MODEL_H
#define MODEL_H

#include <cstdio>
#include <cstdlib>
#include <map>
#include <vector>
#include <tuple>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/intersect.hpp>

#include "material.h"
#include "utils.h"

using Point = std::tuple<glm::vec3, glm::vec3, glm::vec2>;
using Points = std::vector<Point>;
using Face = std::tuple<int, int, int, Material *, unsigned int>;
using Faces = std::vector<Face>;
using Image = std::tuple<int, int, unsigned char *>;
using Images = std::vector<Image>;

enum Option {
	DEFAULT,
	SOFT_SHADOWS,
	DEPTH_OF_FIELD,
	MOTION_BLUR,
	BUMP_MAPPING
};

union ParsingMask {
		float f;
		unsigned int i;
};

struct PointCompare {
	bool operator()(const glm::vec3 &a, const glm::vec3 &b) const
	{ return a.x < b.x || (a.x == b.x && a.y < b.y) || (a.x == b.x && a.y == b.y && a.z < b.z); }
};

struct OctreeNode {
	glm::vec3 minB, maxB;
	int size;
	int *indices;
	OctreeNode *nodes[8];
};

class Model
{
  public:
	Model() = default;

	virtual Model &parse() = 0;
	void collect(Points &global_points, Faces &opaque_faces, Faces &trans_faces, Images &images);
	virtual std::tuple<float, Point, Material *, unsigned int, bool> nearest_intersect(glm::vec3 &p0, glm::vec3 &u, Option option);
	virtual float shadow_attenuation(glm::vec3 &p0, glm::vec3 &u);
	void set_motion(glm::vec3 m);

  protected:
	Points local_points;
	Faces local_faces;
	int offset, width, height;
	unsigned int texture = 0;
	unsigned char *image;
	OctreeNode *top;
	glm::vec3 motion = glm::vec3(0.0f, 0.0f, 0.0f);
};

class PokeBall : public Model
{
  public:
	PokeBall() = default;

	Model &parse() override;
};

class MasterBall : public Model
{
  public:
	MasterBall() = default;

	Model &parse() override;
};

class NetBall : public Model
{
  public:
	NetBall() = default;
	
	Model &parse() override;
};

class TimerBall : public Model
{
  public:
	TimerBall() = default;

	Model &parse() override;
};

class UltraBall : public Model
{
  public:
	UltraBall() = default;

	Model &parse() override;
};

class Table : public Model
{
  public:
	Table() = default;

	Model &parse() override;
};

class MirrorSphere : public Model
{
  public:
	MirrorSphere() = default;

	Model &parse() override;
	std::tuple<float, Point, Material *, unsigned int, bool> nearest_intersect(glm::vec3 &p0, glm::vec3 &u, Option option) override;
	float shadow_attenuation(glm::vec3 &p0, glm::vec3 &u) override;
};

class WoodenSphere : public Model
{
  public:
	WoodenSphere() = default;

	Model &parse() override;
	std::tuple<float, Point, Material *, unsigned int, bool> nearest_intersect(glm::vec3 &p0, glm::vec3 &u, Option option) override;
	float shadow_attenuation(glm::vec3 &p0, glm::vec3 &u) override;
};

class Dice : public Model
{
  public:
	Dice() = default;

	Model &parse() override;
};

class Background : public Model
{
  public:
	Background() = default;

	Model &parse() override;
	std::tuple<float, Point, Material *, unsigned int, bool> nearest_intersect(glm::vec3 &p0, glm::vec3 &u, Option option) override;
};

#endif
