#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "geometry/box3.hpp"

namespace md5 {

struct joint
{
	std::string name;
	int parent;  // -1 ak nema parenta
	glm::vec3 position;  // TODO: v akom space su transformacie ?
	glm::quat orientation;
};

struct weight
{
	int jid;  //!< joint id
	float influence;
	glm::vec3 position;  //!< pozicia vahy v joint-space
};

struct vertex
{
	glm::vec2 uv;
	int wid;  //!< weight id
	int wcount;
};

struct mesh
{
	std::string shader;  //!< nazov textury
	std::vector<weight> weights;
	std::vector<vertex> vertices;
	std::vector<unsigned> indices;
};

struct hierarchy_record
{
	std::string name;
	int parent;
	int flags;
	int start_idx;
};

struct base_frame_record
{
	glm::vec3 position;
	glm::quat orientation;
};

struct animation
{
	int version;
	int frame_rate;
	int animated_component_count;
	std::vector<hierarchy_record> hierarchy;
	std::vector<geom::box3> bounds;
	std::vector<base_frame_record> base_frame;
	std::vector<std::vector<float>> frames;

	animation() {}
	animation(std::string const & fname) {load(fname);}
	void load(std::string const & fname);
};

struct model
{
	int version;
	std::vector<mesh> meshes;
	std::vector<joint> joints;

	model() {}
	model(std::string const & fname) {load(fname);}
	void load(std::string const & fname);
};

void compute_quat_w(glm::quat & q);

}  // md5
