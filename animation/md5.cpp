// citanie md5 modelu
#include <algorithm>
#include <utility>
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
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
#include <boost/filesystem.hpp>
#include <GL/glew.h>
#include "window.hpp"
#include "camera.hpp"
#include "program.hpp"
#include "controllers.hpp"
#include "geometry/box3.hpp"
#include "mesh_exp.hpp"

using std::swap;
using std::string;
using std::vector;
using std::make_pair;
using std::ifstream;
using std::logic_error;
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
using geom::box3;

namespace fs = boost::filesystem;

string model_path = "assets/bob/bob_lamp.md5mesh";
string anim_path = "assets/bob/bob_lamp.md5anim";
string skinning_shader_program = "md5_skinning.glsl";
string solid_shader_path = "solid.glsl";


namespace md5 {

struct joint
{
	string name;
	int parent;  // -1 ak nema parenta
	vec3 position;  // TODO: v akom space su transformacie ?
	quat orientation;
};

struct weight
{
	int jid;  //!< joint id
	float influence;
	vec3 position;  //!< pozicia vahy v joint-space
};

struct vertex
{
	vec2 uv;
	int wid;  //!< weight id
	int wcount;
};

struct mesh
{
	string shader;
	vector<weight> weights;
	vector<vertex> vertices;
	vector<unsigned> indices;
};

struct hierarchy_record
{
	string name;
	int parent;
	int flags;
	int start_idx;
};

struct base_frame_record
{
	vec3 position;
	quat orientation;
};

struct animation
{
	int version;
	int frame_rate;
	int animated_component_count;
	vector<hierarchy_record> hierarchy;
	vector<box3> bounds;
	vector<base_frame_record> base_frame;
	vector<vector<float>> frames;

	animation() {}
	animation(string const & fname) {load(fname);}
	void load(string const & fname);
};

struct model
{
	int version;
	vector<mesh> meshes;
	vector<joint> joints;

	model() {}
	model(string const & fname) {load(fname);}
	void load(string const & fname);
};

void remove_quotes(std::string & str)
{
	 size_t n;
	 while ( ( n = str.find('\"') ) != std::string::npos ) str.erase(n,1);
}

void compute_quat_w(glm::quat & q)
{
	 float t = 1.0f - (q.x * q.x) - (q.y * q.y) - (q.z * q.z);
	 if (t < 0.0f)
		  q.w = 0.0f;
	 else
		  q.w = -sqrt(t);
}

void animation::load(std::string const & fname)
{
	ifstream in{fname};
	if (!in.is_open())
		throw logic_error{"unable to open an animation file '" + fname + "'"};

	fs::path fpath = fname;
	size_t flength = fs::file_size(fpath);

	version = 0;
	frame_rate = 0;
	animated_component_count = 0;
	hierarchy.clear();
	bounds.clear();
	base_frame.clear();
	frames.clear();

	string word, junk;
	unsigned frame_count = 0;
	unsigned joint_count = 0;

	in >> word;
	while (!in.eof())
	{
		if (word == "MD5Version")
		{
			in >> version;
			assert(version == 10);
		}
		else if (word == "commandline")
			in.ignore(flength, '\n');
		else if (word == "numFrames")
		{
			in >> frame_count;
			frames.resize(frame_count);
			in.ignore(flength, '\n');
		}
		else if (word == "numJoints")
		{
			in >> joint_count;
			hierarchy.resize(joint_count);
			in.ignore(flength, '\n');
		}
		else if (word == "frameRate")
		{
			in >> frame_rate;
			in.ignore(flength, '\n');
		}
		else if (word == "numAnimatedComponents")
		{
			in >> animated_component_count;
			in.ignore(flength, '\n');
		}
		else if (word == "hierarchy")
		{
			in >> junk;  // '{'
			for (int i = 0; i < joint_count; ++i)
			{
				hierarchy_record & r = hierarchy[i];
				in >> r.name >> r.parent >> r.flags >> r.start_idx;
				remove_quotes(r.name);
				in.ignore(flength, '\n');
			}
			in >> junk;  // '}'
		}
		else if (word == "bounds")
		{
			in >> junk;  // '{'
			in.ignore(flength, '\n');
			for (int i = 0; i < frame_count; ++i)
			{
				box3 b;
				in >> junk >> b.min_corner().x >> b.min_corner().y >> b.min_corner().z >> junk
					>> junk >> b.max_corner().x >> b.max_corner().y >> b.max_corner().z >> junk;
				bounds.push_back(b);
				in.ignore(flength, '\n');
			}
			in >> junk;  // '}'
			in.ignore(flength, '\n');
		}
		else if (word == "baseframe")
		{
			in >> junk;  // '{'
			in.ignore(flength, '\n');
			for (int i = 0; i < joint_count; ++i)
			{
				base_frame_record f;
				in >> junk >> f.position.x >> f.position.y >> f.position.z >> junk
					>> junk >> f.orientation.x >> f.orientation.y >> f.orientation.z >> junk;
				in.ignore(flength, '\n');
				base_frame.push_back(f);
			}
			in >> junk;  // '}'
			in.ignore(flength, '\n');
		}
		else if (word == "frame")
		{
			vector<float> f;
			unsigned frame_idx;
			in >> frame_idx >> junk;
			in.ignore(flength, '\n');

			f.reserve(animated_component_count);
			for (int i = 0; i < animated_component_count; ++i)
			{
				float d;
				in >> d;
				f.push_back(d);
			}

			assert(frame_idx < frames.size());
			swap(frames[frame_idx], f);

			in >> junk;  // '}'
			in.ignore(flength, '\n');
		}

		in >> word;
	}  // whiel (!in.eof

	assert(hierarchy.size() == joint_count);
	assert(bounds.size() == frame_count);
	assert(base_frame.size() == joint_count);
	assert(frames.size() == frame_count);
}

void model::load(string const & fname)
{
	ifstream in{fname};
	if (!in.is_open())
		throw logic_error{"unable to open a model file '" + fname + "'"};

	size_t flength = fs::file_size(fname);
	assert(flength > 0 && "empty model file");

	joints.clear();
	meshes.clear();

	string word, junk;
	unsigned mesh_count = 0;
	unsigned joint_count = 0;

	in >> word;

	while (!in.eof())
	{
		if (word == "MD5Version")
		{
			in >> version;
			assert(version == 10 && "only version 10 (doom3, quake4) supported");
		}
		else if (word == "commandline")
			in.ignore(flength, '\n');
		else if (word == "numJoints")
		{
			in >> joint_count;
			joints.reserve(joint_count);
		}
		else if (word == "numMeshes")
		{
			in >> mesh_count;
			meshes.reserve(mesh_count);
		}
		else if (word == "joints")
		{
			joint j;
			in >> junk;  // '{'
			for (unsigned i = 0; i < joint_count; ++i)
			{
				in >> j.name >> j.parent
					>> junk >> j.position.x >> j.position.y >> j.position.z >> junk
					>> junk >> j.orientation.x >> j.orientation.y >> j.orientation.z >> junk;
				remove_quotes(j.name);
				compute_quat_w(j.orientation);

				joints.push_back(j);

				in.ignore(flength, '\n');
			}

			in >> junk;  // '}'
		}  // joints
		else if (word == "mesh")
		{
			mesh m;
			in >> junk >> word;  // ignore '{'
			while (word != "}")
			{
				if (word == "shader")
				{
					in >> m.shader;
					remove_quotes(m.shader);
					in.ignore(flength, '\n');
				}
				else if (word == "numverts")
				{
					unsigned numverts;
					in >> numverts;
					in.ignore(flength, '\n');
					for (unsigned i = 0; i < numverts; ++i)
					{
						vertex v;
						in >> junk >> junk  // ignore vert, vertIndex
							>> junk >> v.uv.x >> v.uv.y >> junk
							>> v.wid >> v.wcount;
						in.ignore(flength, '\n');

						m.vertices.push_back(v);
					}
				}
				else if (word == "numtris")
				{
					unsigned numtris;
					in >> numtris;
					in.ignore(flength, '\n');
					for (unsigned i = 0; i < numtris; ++i)
					{
						unsigned v0, v1, v2;
						in >> junk >> junk >> v0 >> v1 >> v2;
						in.ignore(flength, '\n');

						m.indices.push_back(v0);
						m.indices.push_back(v1);
						m.indices.push_back(v2);
					}
				}
				else if (word == "numweights")
				{
					unsigned numweights;
					in >> numweights;
					in.ignore(flength, '\n');
					for (unsigned i = 0; i < numweights; ++i)
					{
						weight w;
						in >> junk >> junk >> w.jid >> w.influence
							>> junk >> w.position.x >> w.position.y >> w.position.z >> junk;
						in.ignore(flength, '\n');

						m.weights.push_back(w);
					}
				}
				else
					in.ignore(flength, '\n');

				in >> word;
			}

			meshes.push_back(m);
		}  // mesh

		in >> word;
	}

	if (meshes.size() != mesh_count || joints.size() != joint_count)
		throw logic_error{string{"model file '"} + fname + "' is corrupted"};
}

}  // md5

namespace gl {

class model
{
public:
	model() {}
	virtual ~model() {}
	virtual void render() const;
	void append_mesh(shared_ptr<mesh> m);

private:
	vector<shared_ptr<mesh>> _meshes;
};

class animation  //!< skeletal animation
{
public:
	unsigned frame_count() const;
	void append_skeleton(vector<mat4> const & skeleton);
	vector<mat4> const & operator[](int idx) const {return _skeletons[idx];}

private:
	vector<vector<mat4>> _skeletons;  //!< skeleton for each frame
};

class animated_model : public model
{
public:
	animated_model() {}
	void update(float dt);
	void render() const override;
	void mesh_animation(animation const & anim) {_anim = anim;}  // model_animation()
	animation const & mesh_animation() const {return _anim;}

	vector<mat4> inverse_bind_pose;

private:
	animation _anim;
};

void animated_model::update(float dt)
{}

void animated_model::render() const
{
	model::render();
}

void model::render() const
{
	for (shared_ptr<mesh> m : _meshes)
		m->render();
}

void model::append_mesh(shared_ptr<mesh> m)
{
	_meshes.push_back(m);
}

unsigned animation::frame_count() const
{
	return _skeletons.size();
}

void animation::append_skeleton(vector<mat4> const & skeleton)
{
	_skeletons.push_back(skeleton);
}

}  //gl

shared_ptr<gl::mesh> create_animated_mesh(md5::model const & model, int mesh_idx)
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

	shared_ptr<gl::mesh> submesh{
		new gl::mesh(vbuf.get(), vertex_size*positions.size(), m.indices.data(), m.indices.size())};

	unsigned stride = (3+2+3+4)*sizeof(GL_FLOAT) + 4*sizeof(GL_INT);
	submesh->append_attribute(gl::attribute{0, 3, GL_FLOAT, stride});  // position
	submesh->append_attribute(gl::attribute{1, 2, GL_FLOAT, stride, 3*sizeof(GL_FLOAT)});  // texcoord
	submesh->append_attribute(gl::attribute{2, 3, GL_FLOAT, stride, (3+2)*sizeof(GL_FLOAT)});  // normal
	// 3 for tangents
	submesh->append_attribute(gl::attribute{4, 4, GL_INT, stride, (3+2+3)*sizeof(GL_FLOAT)});  // joints
	submesh->append_attribute(gl::attribute{5, 4, GL_FLOAT, stride, (3+2+3)*sizeof(GL_FLOAT) + 4*sizeof(GL_UNSIGNED_INT)});  // weights

	assert(stride == vertex_size && "type michmach");

	submesh->__vertbuf.position = positions;
	submesh->__vertbuf.normal = normals;
	submesh->__vertbuf.joints = joints;
	submesh->__vertbuf.weights = weights;

	return submesh;
}

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
	gl::model _mdl;
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
	load_md5mesh(model_path, _mdl);
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

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
//	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	_prog.use();
	_prog.uniform_variable("local_to_screen", local_to_screen);
	_prog.uniform_variable("normal_to_world", normal_to_world);

	// skeleton
	{
		vector<mat4> skel_transforms;
		compute_skeleton_transformations(_animated_skeleton, skel_transforms);
		vector<mat4> const & inverse_bind_pose = _animated.inverse_bind_pose;
		vector<mat4> premultiplied_skeleton;
		for (int i = 0; i < skel_transforms.size(); ++i)
		{
			premultiplied_skeleton.push_back(skel_transforms[i] * inverse_bind_pose[i]);
//			premultiplied_skeleton.push_back(mat4{1});
		}

		_prog.uniform_variable("skeleton", premultiplied_skeleton);
	}

//	_mdl.render();
	_animated.render();

	// normals
	_normvis.use();
	_normvis.uniform_variable("local_to_screen", local_to_screen);
	_normvis.uniform_variable("normal_length", 0.1f);
//	_animated.render();

	// zobraz skeleton
	glDisable(GL_DEPTH_TEST);
	_solid_prog.use();
	_solid_prog.uniform_variable("local_to_screen", local_to_screen);
	_solid_prog.uniform_variable("color", vec3{1,0,0});
	_skeleton.render();

	// skeleton s animacie
	_solid_prog.uniform_variable("color", vec3{0,0,1});
	_animated_skeleton_mesh.render();

	base::display();
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

void scene_window::load_animated_md5mesh(string const & mesh_file, string const & anim_file)
{
	_md5_mdl.load(mesh_file);
	for (int i = 0; i < _md5_mdl.meshes.size(); ++i)
		_animated.append_mesh(create_animated_mesh(_md5_mdl, i));

	// inverse bind pose
	for (md5::joint const & j : _md5_mdl.joints)
	{
		mat4 T = inverse(translate(mat4{1}, j.position) * mat4_cast(j.orientation));
		_animated.inverse_bind_pose.push_back(T);
	}

	// skeleton mesh
	_skeleton = create_skeleton_mesh(_md5_mdl.joints);

	_md5_anim.load(anim_file);

//	gl::animation anim;
//	for (int i = 0; i < _md5_anim.frames.size(); ++i)
//	{
//		vector<mat4> skel;  // TODO: pre kazdy frame ma skeleton rovnaky size
//		create_skeleton(_md5_anim, i, skel);
//		anim.append_skeleton(skel);
//	}

//	_animated.mesh_animation(anim);
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
