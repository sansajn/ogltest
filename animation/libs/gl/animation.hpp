#pragma once
#include <memory>
#include <vector>
#include <string>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include "mesh.hpp"

namespace gl {

class skeletal_animation
{
public:
	struct bone
	{
		glm::vec3 position;
		glm::quat orientation;
	};

	skeletal_animation() {}
	skeletal_animation(std::string const & fname);
	void from_file(std::string const & fname);
	unsigned frame_count() const;
	unsigned frame_rate() const;
	std::vector<bone> const & operator[](int n) const;
	std::vector<bone> skeleton(float frame) const;  // vrati interpolovany skeleton
	std::vector<bone> nearest_skeleton(float frame) const;
	std::vector<bone> lerp_skeleton(float frame) const;
	std::vector<bone> slerp_skeleton(float frame) const;

	// TODO: spav s toho nekopirovatelnu strukturu
//	void operator=(skeletal_animation const &) = delete;
//	skeletal_animation(skeletal_animation const &) = delete;

private:
	unsigned _frame_rate;
	std::vector<std::vector<bone>> _skeletons;  //!< jeden skeleton pre kazdy frame
};

// TODO: pravdepodobne bude scenar v ktorom budem mat jeden model a niekolko animacii
class animated_model : public model
{
public:
	animated_model() {}
	void update(float dt);
	void assign_animation(skeletal_animation && a);
	void assign_inverse_bind_pose(std::vector<glm::mat4> && pose);
	std::vector<glm::mat4> const & skeleton() const;  //!< vrati aktualnu kostru (ako maticove transformacie pripravene pre shader)
	skeletal_animation const & animation() const;
	bool has_animation() const {return _has_animation;}

private:
	bool _has_animation = false;
	skeletal_animation _anim;
	mutable std::vector<glm::mat4> _curr_skel_transfs;
	float _anim_time = 0;
	std::vector<glm::mat4> _inverse_bind_pose;
};

animated_model animated_model_from_file(std::string const & mesh_file, std::string const & anim_file);

}  // gl
