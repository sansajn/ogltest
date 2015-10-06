// citanie md5 modelu
#include <algorithm>
#include <utility>
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <cassert>
#include <iostream>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <GL/glew.h>
#include "window.hpp"
#include "camera.hpp"
#include "program.hpp"
#include "controllers.hpp"
#include "mesh.hpp"
#include "md5loader.hpp"
#include "animation.hpp"
#include "model.hpp"

using std::swap;
using std::string;
using std::vector;
using std::make_pair;
using std::move;
using std::shared_ptr;
using std::unique_ptr;
using std::max;
using std::min;
using std::cout;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::ivec4;
using glm::mat3;
using glm::mat4;
using glm::cross;
using glm::normalize;
using glm::radians;
using glm::inverseTranspose;
using glm::inverse;
using glm::rotate;
using glm::translate;
using glm::scale;
using glm::mat4_cast;

using std::string;
using gl::skeletal_animation;

// quake 4 blaster
string model_path = "assets/blaster/view.md5mesh";

string anim_paths[] = {
	"assets/blaster/big_recoil.md5anim",
	"assets/blaster/charge_up.md5anim",
	"assets/blaster/fire.md5anim",
	"assets/blaster/fire2.md5anim",
	"assets/blaster/flashlight.md5anim",
	"assets/blaster/idle.md5anim",
	"assets/blaster/lower.md5anim",
	"assets/blaster/raise.md5anim"
};

string skinned_shader_program = "shaders/bump_skinned.glsl";
string axis_shader_path = "shaders/colored.glsl";
string light_shader_path = "shaders/solid.glsl";

static shared_ptr<gl::mesh> create_animated_textured_mesh(md5::model const & mdl, int mesh_idx);

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
	void append_animation(skeletal_animation && a);
	void animation_sequence(vector<unsigned> const & s, repeat_mode m = repeat_mode::loop);
	state animation_state() const;
	void assign_inverse_bind_pose(std::vector<glm::mat4> && pose);
	std::vector<glm::mat4> const & skeleton() const;  //!< vrati aktualnu kostru (ako maticove transformacie pripravene pre shader)
	skeletal_animation const & animation() const;
	bool has_animation() const;

private:
	void compute_skeleton_transformations(vector<skeletal_animation::bone> const & skel);

	vector<skeletal_animation> _anims;
	std::vector<unsigned> _anim_seq;
	mutable std::vector<glm::mat4> _curr_skel_transfs;
	float _anim_time = 0;
	std::vector<glm::mat4> _inverse_bind_pose;
	unsigned _anim_seq_idx = 0;  //!< current index in animation sequence
	state _anim_state;
	repeat_mode _repeat;
};

static animated_textured_model animated_textured_model_from_file(string const & mesh_file);

class player    // player_object
{
public:
	enum {  // animationas
		big_recoil_animation,
		charge_up_animation,
		fire_animation,
		fire2_animation,
		flashlight_animation,
		idle_animation,
		lower_animation,
		raise_animation
	};

	enum class state {
		idle,
		fire,
	};

	void init();
	void update(float dt);
	void render(shader::program & prog);

	// states
	void fire();
	void idle();

	// docastne
	std::vector<glm::mat4> const & skeleton() const {return _mdl.skeleton();}

private:
	animated_textured_model _mdl;
	state _state;
};

void player::init()
{
	_mdl = animated_textured_model_from_file(model_path);

	for (string const & anim_path : anim_paths)
		_mdl.append_animation(skeletal_animation{anim_path});

	_mdl.animation_sequence(vector<unsigned>{idle_animation});

	_state = state::idle;
}

void player::update(float dt)
{
	// state update
	if (_state == state::fire)
	{
		if (_mdl.animation_state() == animated_textured_model::state::done)
			idle();
	}

	_mdl.update(dt);
}

void player::render(shader::program & prog)
{
	_mdl.render(prog);
}

void player::fire()
{
	_state = state::fire;
	_mdl.animation_sequence(vector<unsigned>{fire_animation}, animated_textured_model::repeat_mode::once);  // TODO: specializovana funkcia pre sekvencie dlzky 1
}

void player::idle()
{
	_state = state::idle;
	_mdl.animation_sequence(vector<unsigned>{idle_animation}, animated_textured_model::repeat_mode::loop);
}

void animated_textured_model::update(float dt)
{
	if (_anim_seq.empty() || !has_animation() || _anim_state == state::done)
		return;

	bool done = false;

	_anim_time += dt;
	if (_anim_time >= animation().frame_count() / float(animation().frame_rate()))
	{
		// next animation in sequence
		_anim_time = 0;
		++_anim_seq_idx;
		if (_anim_seq_idx >= _anim_seq.size())  // end of animation sequence
		{
			if (_repeat == repeat_mode::loop)
				_anim_seq_idx = 0;  // again
			else
				done = true;  // koniec animacie
		}
	}

	float	frame = 0;
	if (!done)
		frame = animation().frame_rate() * _anim_time;
	else  // nastav animaciu na posledny frame
	{
		_anim_seq_idx = _anim_seq.size() - 1;
		frame = animation().frame_count() - 1;
		_anim_state = state::done;
	}

	vector<skeletal_animation::bone> const & skel = animation().skeleton(frame);

	assert(frame < animation().frame_count() && "out of frames");
	assert(skel.size() == _inverse_bind_pose.size() && "bind pose not match");

	compute_skeleton_transformations(skel);
}

void animated_textured_model::compute_skeleton_transformations(vector<skeletal_animation::bone> const & skel)
{
	_curr_skel_transfs.clear();
	_curr_skel_transfs.reserve(skel.size());
	for (unsigned i = 0; i < skel.size(); ++i)
	{
		_curr_skel_transfs.push_back(
			translate(mat4{1}, skel[i].position) * mat4_cast(skel[i].orientation) * _inverse_bind_pose[i]);
	}
}

void animated_textured_model::animation_sequence(vector<unsigned> const & s, repeat_mode m)
{
	assert(s.size() > 0 && "empty sequence");
	_anim_seq = s;
	_repeat = m;
	_anim_seq_idx = 0;
	_anim_time = 0;
	_anim_state = state::in_progress;
	compute_skeleton_transformations(animation().skeleton(0));
	assert(_anims.size() > 0 && "ziadna animacia");
}

animated_textured_model::state animated_textured_model::animation_state() const
{
	return _anim_state;
}

void animated_textured_model::append_animation(skeletal_animation && a)
{
	_anims.push_back(move(a));
}

void animated_textured_model::assign_inverse_bind_pose(vector<mat4> && pose)
{
	_inverse_bind_pose = move(pose);
}

vector<mat4> const & animated_textured_model::skeleton() const
{
	if (_anim_seq.empty() && _curr_skel_transfs.empty())
		_curr_skel_transfs.assign(100, mat4{1});  // TODO: 100 mam zadratovane v shadery

	return _curr_skel_transfs;
}

skeletal_animation const & animated_textured_model::animation() const
{
	return _anims[_anim_seq[_anim_seq_idx]];
}

bool animated_textured_model::has_animation() const
{
	return !_anims.empty();
}

bool diffuse_exists(char const * path)
{
	return texman.exists(string{path} + "_d.tga");
}

bool local_normal_exists(char const * path)
{
	return texman.exists(string{path} + "_local.tga");
}

bool height_exists(char const * path)
{
	return texman.exists(string{path} + "_h.tga");
}

bool specular_exists(char const * path)
{
	return texman.exists(string{path} + "_s.tga");
}

bool glow_exists(char const * path)
{
	return texman.exists(string{path} + "_glow.tga");
}

animated_textured_model animated_textured_model_from_file(string const & mesh_file)
{
	animated_textured_model result;

	md5::model mdl{mesh_file};
	for (int i = 0; i < mdl.meshes.size(); ++i)
	{
		string texture_id = mdl.meshes[i].shader;

		int type = textured_model::none_texture;
		if (diffuse_exists(texture_id.c_str()))
			type |= textured_model::diffuse_texture;
		if (local_normal_exists(texture_id.c_str()))
			type |= textured_model::normal_texture;
		if (height_exists(texture_id.c_str()))
			type |= textured_model::height_texture;
		if (specular_exists(texture_id.c_str()))
			type |= textured_model::specular_texture;
		if (glow_exists(texture_id.c_str()))
			type |= textured_model::glow_texture;

		result.append_mesh(create_animated_textured_mesh(mdl, i), mdl.meshes[i].shader, type);
	}

	vector<mat4> inverse_bind_pose;
	inverse_bind_pose.reserve(mdl.joints.size());

	for (md5::joint const & j : mdl.joints)  // compute inverse bind pose
	{
		inverse_bind_pose.push_back(
			inverse(translate(mat4{1}, j.position) * mat4_cast(j.orientation)));
	}

	result.assign_inverse_bind_pose(move(inverse_bind_pose));

	return result;
}


// TODO: optimalizuj
shared_ptr<gl::mesh> create_animated_textured_mesh(md5::model const & mdl, int mesh_idx)
{
	md5::mesh const & m = mdl.meshes[mesh_idx];

	// pozicie
	vector<vec3> positions;

	for (md5::vertex const & v : m.vertices)
	{
		vec3 pos{0};
		for (int i = 0; i < v.wcount; ++i)
		{
			md5::weight const & w = m.weights[v.wid + i];
			md5::joint const & j = mdl.joints[w.jid];
			pos += w.influence * (j.position + (j.orientation * w.position));
		}
		positions.push_back(pos);
	}

	// normaly
	vector<vec3> normals{positions.size()};

	for (int i = 0; i < m.indices.size(); i += 3)
	{
		vec3 & p0 = positions[m.indices[i]];
		vec3 & p1 = positions[m.indices[i+1]];
		vec3 & p2 = positions[m.indices[i+2]];

		vec3 n = cross(p1 - p0, p2 - p0);
		n = normalize(n);  // TODO: robim sucet normal ktory potom normalizujem tzn. netrea normalizovat

		normals[m.indices[i]] += n;
		normals[m.indices[i+1]] += n;
		normals[m.indices[i+2]] += n;
	}

	for (vec3 & n : normals)
		n = normalize(n);

	// joint indices (four per vertex)
	vector<ivec4> joints;
	for (md5::vertex const & v : m.vertices)
	{
		assert(v.wcount < 5 && "only 4 weights per vertex supported");

		ivec4 ids{0};
		for (int i = 0; i < min(v.wcount, 4); ++i)
			ids[i] = m.weights[v.wid + i].jid;

		joints.push_back(ids);
	}

	// weigths (four per vertex)
	vector<vec4> weights;
	for (md5::vertex const & v : m.vertices)
	{
		assert(v.wcount < 5 && "only 4 weights per vertex supported");

		vec4 influence{0};
		for (int i = 0; i < min(v.wcount, 4); ++i)
			influence[i] = m.weights[v.wid + i].influence;

		weights.push_back(influence);
	}

	// vytvor mesh buffer
	unsigned vertex_size = (3+2+3+4)*sizeof(float) + 4*sizeof(int);  // position:3, uv:2, normal:3, joint:4, weight:4
	unique_ptr<uint8_t[]> vbuf{new uint8_t[vertex_size*positions.size()]};
	float * fbuf = (float *)vbuf.get();

	for (int i = 0; i < positions.size(); ++i)
	{
		vec3 & p = positions[i];
		*fbuf++ = p.x;
		*fbuf++ = p.y;
		*fbuf++ = p.z;

		vec2 const & uv = m.vertices[i].uv;
		*fbuf++ = uv.x;
		*fbuf++ = uv.y;

		vec3 & n = normals[i];
		*fbuf++ = n.x;
		*fbuf++ = n.y;
		*fbuf++ = n.z;

		int * ubuf = (int *)(fbuf);
		ivec4 & ids = joints[i];
		*ubuf++ = ids.x;
		*ubuf++ = ids.y;
		*ubuf++ = ids.z;
		*ubuf++ = ids.w;

		fbuf = (float *)(ubuf);
		vec4 & influence = weights[i];
		*fbuf++ = influence.x;
		*fbuf++ = influence.y;
		*fbuf++ = influence.z;
		*fbuf++ = influence.w;
	}

	shared_ptr<gl::mesh> result{
		new gl::mesh(vbuf.get(), vertex_size*positions.size(), m.indices.data(), m.indices.size())};

	unsigned stride = (3+2+3+4)*sizeof(GL_FLOAT) + 4*sizeof(GL_INT);
	result->append_attribute(gl::attribute{0, 3, GL_FLOAT, stride});  // position
	result->append_attribute(gl::attribute{1, 2, GL_FLOAT, stride, 3*sizeof(GL_FLOAT)});  // texcoord
	result->append_attribute(gl::attribute{2, 3, GL_FLOAT, stride, (3+2)*sizeof(GL_FLOAT)});  // normal
	// 3 for tangents
	result->append_attribute(gl::attribute{4, 4, GL_INT, stride, (3+2+3)*sizeof(GL_FLOAT)});  // joints
	result->append_attribute(gl::attribute{5, 4, GL_FLOAT, stride, (3+2+3)*sizeof(GL_FLOAT) + 4*sizeof(GL_UNSIGNED_INT)});  // weights

	assert(stride == vertex_size && "type michmach");

	return result;
}


class scene_window : public ui::glut_pool_window
{
public:
	using base = ui::glut_pool_window;

	scene_window();
	void update(float dt) override;
	void display() override;
	void input(float dt) override;

private:
	gl::camera _cam;
	vector<shared_ptr<gl::camera_controller>> _cam_ctrls;
	shader::program _prog;

	player _player;

	// debug
	mesh _axis;
	shader::program _axis_prog;
	mesh _light;
	shader::program _light_prog;
};

scene_window::scene_window()
{
	_player.init();

	_cam = gl::camera{radians(70.0f), aspect_ratio(), 0.01, 1000};
	_cam.look_at(vec3{1,0,0});
	_cam_ctrls.push_back(shared_ptr<gl::free_move<scene_window>>{new gl::free_move<scene_window>{_cam, *this, 0.1}});
	_cam_ctrls.push_back(shared_ptr<gl::free_look<scene_window>>{new gl::free_look<scene_window>{_cam, *this}});

	_prog.from_file(skinned_shader_program);

	_axis = gl::make_axis();
	_axis_prog.from_file(axis_shader_path);
	_light = gl::make_sphere();
	_light_prog.from_file(light_shader_path);

	glClearColor(0,0,0,1);
}

void scene_window::update(float dt)
{
	base::update(dt);
	_player.update(dt);
}

void scene_window::display()
{
	mat4 M = mat4{1};
	M = rotate(M, radians(-90.0f), vec3{1, 0, 0});
	mat4 world_to_camera = _cam.view();
	mat4 local_to_screen = _cam.projection() * world_to_camera * M;
	mat3 normal_to_camera = mat3{inverseTranspose(world_to_camera * M)};

	vec3 light_pos = vec3{5,5,-5};

	_prog.use();
	_prog.uniform_variable("local_to_world", M);
	_prog.uniform_variable("world_to_camera", world_to_camera);
	_prog.uniform_variable("local_to_screen", local_to_screen);
	_prog.uniform_variable("normal_to_camera", normal_to_camera);
	_prog.uniform_variable("light.direction", normalize(light_pos));
	_prog.uniform_variable("skeleton", _player.skeleton());

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	_player.render(_prog);

	// light
	mat4 M_light = translate(mat4{1}, light_pos);
	M_light = scale(M_light, vec3{0.1, 0.1, 0.1});
	_light_prog.use();
	_light_prog.uniform_variable("color", vec3{1,1,0});  // yellow
	_light_prog.uniform_variable("local_to_screen", _cam.view_projection() * M_light);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	_light.render();

	// axis
	_axis_prog.use();
	_axis_prog.uniform_variable("local_to_screen", _cam.view_projection());
	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	_axis.render();

	base::display();
}

void scene_window::input(float dt)
{
	for (auto ctrl : _cam_ctrls)
		ctrl->input(dt);

	if (in.key_up(' '))  // TODO: ako na fire-rate ?
		_player.fire();

	base::input(dt);
}


int main(int argc, char * argv[])
{
	texman.root_path("assets/blaster");
	scene_window w;
	w.start();
	return 0;
}
