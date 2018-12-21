#pragma once
#include <memory>
#include <vector>
#include <string>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include "gles2/model_loader_gles2.hpp"

namespace gles2 {

//! kosterna animacia, podporuje md5anim format
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

private:
	unsigned _frame_rate;
	std::vector<std::vector<bone>> _skeletons;
};


/*! Animovany model (kosterna animacia).
\code
animated_model mdl = animated_model_from_file(model_path);
for (string const & anim_path : anim_paths)
	mdl.append_animation(skeletal_animation{anim_path});
mdl.animation_sequence(vector<unsigned>{1,2,3});  // spusti 2, 3 a 4 animaciu
\endcode */
class animated_model : public model
{
public:
	enum class repeat_mode {  // repeat mode
		once,
		loop
	};

	enum class state {  // animation state, TODO: toto stasi nahradit funkciou is_done()/in_progress()/is_playing()
		in_progress,
		done
	};

	animated_model() {}  //! neinicializovny model
	void update(float dt);
	void append_animation(skeletal_animation && a);
	void animation_sequence(std::vector<unsigned> const & s, repeat_mode m = repeat_mode::loop);
	state animation_state() const;
	void assign_inverse_bind_pose(std::vector<glm::mat4> && pose);
	std::vector<glm::mat4> const & skeleton() const;  //!< vrati aktualnu kostru (ako maticove transformacie pripravene pre shader)
	skeletal_animation const & animation() const;
	bool has_animation() const;

private:
	void compute_skeleton_transformations(std::vector<skeletal_animation::bone> const & skel);

	std::vector<skeletal_animation> _anims;
	std::vector<unsigned> _anim_seq;
	mutable std::vector<glm::mat4> _curr_skel_transfs;
	float _anim_time = 0;
	std::vector<glm::mat4> _inverse_bind_pose;
	unsigned _anim_seq_idx = 0;  //!< current index in animation sequence
	state _anim_state;
	repeat_mode _repeat;
};

//! \note podpora zatial iba pre md5 format
animated_model animated_model_from_file(std::string const & model_file, model_loader_parameters const & params = model_loader_parameters{});

}  // gles2
