#pragma once
#include <map>
#include <vector>
#include <string>
#include <memory>
#include "gl/texture.hpp"
#include "gl/mesh.hpp"
#include "gl/program.hpp"
#include "gl/animation.hpp"

using gl::mesh;

class texture_manager
{
public:
	// get()
	// find()
	std::shared_ptr<texture2d> find(std::string const & key);  // texture_name
	void root_path(std::string const & path) {_root_path = path;}
	bool exists(std::string const & texture_name) const;

private:
	std::string get_texture_filename(std::string const & key) const;

	std::string _root_path;
	std::map<std::string, std::shared_ptr<texture2d>> _textures;
};

extern texture_manager texman;  //  global texture manager


class textured_model
{
public:
	enum texture_type
	{
		none_texture = 0,
		diffuse_texture = 1,
		normal_texture = 2,
		height_texture = 4,
		specular_texture = 8,
		glow_texture = 16
	};

	textured_model() {}
	textured_model(textured_model && other);
	void render(shader::program & prog);
	void append_mesh(std::shared_ptr<mesh> m, std::string const & texture_id, int texture_compound = diffuse_texture);
	void operator=(textured_model && other);

	textured_model(textured_model const &) = delete;
	void operator=(textured_model const &) = delete;

private:
	std::vector<std::string> _texture_ids;
	std::vector<int> _texture_compound;  // texture_type
	std::vector<std::shared_ptr<mesh>> _meshes;
};

textured_model textured_model_from_file(std::string const & fname);


// light, texture, animation
class animated_textured_model : public textured_model
{
public:
	enum class repeat_mode {  // repeat mode
		once,
		loop
	};

	enum class state {  // animation state
		in_progress,
		done
	};

	animated_textured_model() {}
	void update(float dt);
	void append_animation(gl::skeletal_animation && a);
	void animation_sequence(std::vector<unsigned> const & s, repeat_mode m = repeat_mode::loop);
	state animation_state() const;
	void assign_inverse_bind_pose(std::vector<glm::mat4> && pose);
	std::vector<glm::mat4> const & skeleton() const;  //!< vrati aktualnu kostru (ako maticove transformacie pripravene pre shader)
	gl::skeletal_animation const & animation() const;
	bool has_animation() const;

private:
	void compute_skeleton_transformations(std::vector<gl::skeletal_animation::bone> const & skel);

	std::vector<gl::skeletal_animation> _anims;
	std::vector<unsigned> _anim_seq;
	mutable std::vector<glm::mat4> _curr_skel_transfs;
	float _anim_time = 0;
	std::vector<glm::mat4> _inverse_bind_pose;
	unsigned _anim_seq_idx = 0;  //!< current index in animation sequence
	state _anim_state;
	repeat_mode _repeat;
};

animated_textured_model animated_textured_model_from_file(std::string const & mesh_file);
