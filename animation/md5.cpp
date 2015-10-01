// citanie md5 modelu
#include <algorithm>
#include <utility>
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <cassert>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/common.hpp>
#include <glm/gtc/quaternion.hpp>
#include <GL/glew.h>
#include "window.hpp"
#include "camera.hpp"
#include "program.hpp"
#include "controllers.hpp"
#include "mesh.hpp"
#include "md5loader.hpp"
#include "animation.hpp"

using std::swap;
using std::string;
using std::vector;
using std::make_pair;
using std::shared_ptr;
using std::unique_ptr;
using std::max;
using std::min;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::ivec4;
using glm::quat;
using glm::mat3;
using glm::mat4;
using glm::cross;
using glm::normalize;
using glm::radians;
using glm::inverseTranspose;
using glm::inverse;
using glm::rotate;
using glm::translate;
using glm::mat4_cast;

string model_path = "assets/bob/bob_lamp.md5mesh";
string anim_path = "assets/bob/bob_lamp.md5anim";
string skinning_shader_program = "md5_skinning.glsl";
string solid_shader_path = "solid.glsl";

shared_ptr<gl::mesh> create_mesh(md5::model const & model, int mesh_idx)
{
	md5::mesh const & m = model.meshes[mesh_idx];

	// pozicie
	vector<vec3> positions;

	for (md5::vertex const & v : m.vertices)
	{
		vec3 pos{0};
		for (int i = 0; i < v.wcount; ++i)
		{
			md5::weight const & w = m.weights[v.wid + i];
			md5::joint const & j = model.joints[w.jid];
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

		vec3 n = cross(p2 - p0, p1 - p0);
		n = normalize(n);  // TODO: robim sucet normal ktory potom normalizujem tzn. netrea normalizovat

		normals[m.indices[i]] += n;
		normals[m.indices[i+1]] += n;
		normals[m.indices[i+2]] += n;
	}

	for (vec3 & n : normals)
		n = normalize(n);

	// vytvor mesh
	vector<float> vbuf;
	vbuf.reserve((3+2+3)*positions.size());  // position:3, uv:2, normal:3

	for (int i = 0; i < positions.size(); ++i)
	{
		vec3 & p = positions[i];
		vbuf.push_back(p.x);
		vbuf.push_back(p.y);
		vbuf.push_back(p.z);

		vec2 const & uv = m.vertices[i].uv;
		vbuf.push_back(uv.x);
		vbuf.push_back(uv.y);

		vec3 & n = normals[i];
		vbuf.push_back(n.x);
		vbuf.push_back(n.y);
		vbuf.push_back(n.z);
	}

	shared_ptr<gl::mesh> submesh{
		new gl::mesh(vbuf.data(), vbuf.size()*sizeof(float), m.indices.data(), m.indices.size())};

	unsigned stride = (3+2+3)*sizeof(GL_FLOAT);
	submesh->append_attribute(gl::attribute{0, 3, GL_FLOAT, stride});  // position
	submesh->append_attribute(gl::attribute{1, 2, GL_FLOAT, stride, 3*sizeof(GL_FLOAT)});  // uv
	submesh->append_attribute(gl::attribute{2, 3, GL_FLOAT, stride, (3+2)*sizeof(GL_FLOAT)});  // normal

	return submesh;
}

struct bone
{
	int parent_idx;
	vec3 position;
	quat orientation;
};

struct skeleton
{
	vector<bone> bones;
};

void create_skeleton(md5::animation const & anim, unsigned nframe, skeleton & result)
{
	result.bones.resize(anim.hierarchy.size());

	vector<float> const & frame = anim.frames[nframe];

	for (int i = 0; i < anim.hierarchy.size(); ++i)
	{
		md5::hierarchy_record const & h = anim.hierarchy[i];

		bone & b = result.bones[i];
		b.position = anim.base_frame[i].position;
		b.orientation = anim.base_frame[i].orientation;
		b.parent_idx = h.parent;

		int idx = h.start_idx;
		if (h.flags & 1)
			b.position.x = frame[idx++];
		if (h.flags & 2)
			b.position.y = frame[idx++];
		if (h.flags & 4)
			b.position.z = frame[idx++];
		if (h.flags & 8)
			b.orientation.x = frame[idx++];
		if (h.flags & 16)
			b.orientation.y = frame[idx++];
		if (h.flags & 32)
			b.orientation.z = frame[idx++];

		md5::compute_quat_w(b.orientation);

		if (h.parent > -1)
		{
			assert(h.parent < result.bones.size());
			assert(h.parent < i && "parent was not yet evaluated");

			bone const & b_parent = result.bones[h.parent];
			b.position = b_parent.position + (b_parent.orientation * b.position);
			b.orientation = normalize(b_parent.orientation * b.orientation);
		}
	}
}

void compute_skeleton_transformations(skeleton const & skel, vector<mat4> & result)
{
	for (bone const & b : skel.bones)
		result.push_back(translate(mat4{1}, b.position) * mat4_cast(b.orientation));
}

void load_md5mesh(string const & fname, gl::model & result)
{
	md5::model mdl{fname};
	for (int i = 0; i < mdl.meshes.size(); ++i)
		result.append_mesh(create_mesh(mdl, i));
}

gl::mesh create_skeleton_mesh(skeleton const & s);
gl::mesh create_skeleton_mesh(vector<md5::joint> const & joints);

class scene_window : public ui::glut_pool_window
{
public:
	using base = ui::glut_pool_window;

	scene_window();
	void update(float dt) override;
	void display() override;
	void input(float dt) override;

private:
	void load_animated_md5mesh(string const & mesh_file, string const & anim_file);

	gl::camera _cam;
	vector<shared_ptr<gl::camera_controller>> _cam_ctrls;
	shader::program _prog;
	shader::program _normvis;
	shader::program _solid_prog;

	// model stuff
	gl::animated_model _animated;
	gl::mesh _skeleton;
	md5::animation _md5_anim;
	md5::model _md5_mdl;
	skeleton _animated_skeleton;
	gl::mesh _animated_skeleton_mesh;

	float _anim_time = 0.0;
};

scene_window::scene_window()
	: _cam{radians(70.0f), aspect_ratio(), 0.01, 1000}
{
	load_animated_md5mesh(model_path, anim_path);

	_cam.position = vec3{0,3,10};
	_cam_ctrls.push_back(shared_ptr<gl::free_move<scene_window>>{new gl::free_move<scene_window>{_cam, *this, 0.1}});
	_cam_ctrls.push_back(shared_ptr<gl::free_look<scene_window>>{new gl::free_look<scene_window>{_cam, *this}});

	_prog.from_file(skinning_shader_program);
	_normvis.from_file("normvis.glsl");
	_solid_prog.from_file(solid_shader_path);

	glClearColor(0,0,0,1);
}

void scene_window::update(float dt)
{
	base::update(dt);

	_anim_time += dt;
	unsigned nframe = (unsigned)(_md5_anim.frame_rate * _anim_time);
	if (nframe >= _md5_anim.frames.size())
	{
		nframe = 0;
		_anim_time = 0;
	}

	_animated_skeleton.bones.clear();
	create_skeleton(_md5_anim, nframe, _animated_skeleton);
	_animated_skeleton_mesh = create_skeleton_mesh(_animated_skeleton);

	_animated.update(dt);
}

void scene_window::display()
{
	mat4 M = mat4{1};
	M = rotate(mat4{1}, radians(-90.0f), vec3{1,0,0});
	mat4 local_to_screen = _cam.view_projection() * M;
	mat3 normal_to_world = mat3{inverseTranspose(M)};

//	glFrontFace(GL_CW);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
//	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	_prog.use();
	_prog.uniform_variable("local_to_screen", local_to_screen);
	_prog.uniform_variable("normal_to_world", normal_to_world);
	_prog.uniform_variable("skeleton", _animated.skeleton());
	_animated.render();

	// normals
//	_normvis.use();
//	_normvis.uniform_variable("local_to_screen", local_to_screen);
//	_normvis.uniform_variable("normal_length", 0.1f);
//	_animated.render();

	// zobraz skeleton
	glDisable(GL_DEPTH_TEST);
	_solid_prog.use();
	_solid_prog.uniform_variable("local_to_screen", local_to_screen);
	_solid_prog.uniform_variable("color", vec3{1,0,0});
	_skeleton.render();

	// animovany skeleton
	_solid_prog.uniform_variable("color", vec3{0,0,1});
	_animated_skeleton_mesh.render();

	base::display();
}

void scene_window::load_animated_md5mesh(string const & mesh_file, string const & anim_file)
{
	_animated = gl::animated_model_from_file(mesh_file, anim_file);

	// skeleton vizualization
	_md5_mdl.load(mesh_file);
	_md5_anim.load(anim_file);
	_skeleton = create_skeleton_mesh(_md5_mdl.joints);
}

void scene_window::input(float dt)
{
	for (auto ctrl : _cam_ctrls)
		ctrl->input(dt);

	base::input(dt);
}


int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}

gl::mesh create_skeleton_mesh(vector<md5::joint> const & joints)
{
	// indices
	vector<unsigned> indices;
	for (int j = 0; j < joints.size(); ++j)
	{
		int i = joints[j].parent;
		if (i == -1)
			continue;
		indices.push_back(i);
		indices.push_back(j);
	}

	// positions
	vector<vec3> positions;
	for (md5::joint const & j : joints)
		positions.push_back(j.position);

	gl::mesh result(positions.data(), positions.size()*sizeof(vec3), indices.data(), indices.size());
	result.append_attribute(gl::attribute{0, 3, GL_FLOAT, 3*sizeof(GL_FLOAT)});
	result.draw_mode(GL_LINES);
	return result;
}

gl::mesh create_skeleton_mesh(skeleton const & s)
{
	// positions
	vector<vec3> positions;
	for (bone const & b : s.bones)
		positions.push_back(b.position);

	// indices
	vector<unsigned> indices;
	for (int j = 0; j < s.bones.size(); ++j)
	{
		int i = s.bones[j].parent_idx;
		if (i == -1)
			continue;
		indices.push_back(i);
		indices.push_back(j);
	}

	gl::mesh result(positions.data(), positions.size()*sizeof(vec3), indices.data(), indices.size());
	result.append_attribute(gl::attribute{0, 3, GL_FLOAT, 3*sizeof(GL_FLOAT)});
	result.draw_mode(GL_LINES);
	return result;
}
