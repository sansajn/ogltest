// render md5 modelu v opengl 1.x mode
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <memory>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/common.hpp>
#include <boost/filesystem.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "camera.hpp"
#include "controllers.hpp"
#include "window.hpp"

using std::string;
using std::vector;
using std::ifstream;
using std::cerr;
using std::shared_ptr;
using glm::vec3;
using glm::vec2;
using glm::quat;
using glm::mat4;
using glm::cross;
using glm::normalize;
using glm::translate;
using glm::rotate;
using glm::radians;
using glm::lerp;
using glm::mix;
namespace fs = boost::filesystem;

string model_path = "assets/bob/bob_lamp.md5mesh";
string anim_path = "assets/bob/bob_lamp.md5anim";

//string model_path = "assets/doctor/doctor.md5mesh";
//string anim_path = "assets/doctor/walking_tablegui_1.md5anim";


class md5animation
{
public:
	md5animation();
	bool load_animation(std::string const & fname);
	void update(float dt);
	void render();

	struct joint_info
	{
		string name;
		int parent, flags, start_index;
	};

	struct bound
	{
		vec3 min_corner, max_corner;
	};

	struct base_frame
	{
		vec3 pos;
		quat orient;
	};

	struct frame_data
	{
		int frame_idx;
		vector<float> data;
	};

	struct skeleton_joint
	{
		int parent = -1;
		vec3 pos;
		quat orient;

		skeleton_joint() {}
		skeleton_joint(base_frame const & f) : pos{f.pos}, orient{f.orient} {}
	};

	struct frame_skeleton
	{
		vector<skeleton_joint> joints;
	};

	frame_skeleton const & skeleton() const {return _animated_skeleton;}
	int joint_count() const {return _joint_count;}
	joint_info const & joint_info_at(unsigned idx) const {return _joint_infos[idx];} 

private:
	void build_frame_skeleton(frame_data const & f);
	void interpolate_skeletons(frame_skeleton const & a, frame_skeleton const & b, float interp, frame_skeleton & result);

	vector<joint_info> _joint_infos;
	vector<bound> _bounds;
	vector<base_frame> _base_frames;
	vector<frame_data> _frames;
	vector<frame_skeleton> _skeletons;
	frame_skeleton _animated_skeleton;

	int _version;
	int _frame_count, _joint_count, _animated_component_count;
	int _frame_rate;
	float _anim_duration, _frame_duration, _anim_time;
};

class md5model
{
public:
	md5model();
	bool load_model(string const & fname);
	bool load_animation(string const & fname);
	void update(float dt);
	void render();

private:
	struct vertex
	{
		vec3 pos, normal;
		vec2 tex0;
		int start_weight;
		int weight_count;
	};

	struct triangle
	{
		int v0, v1, v2;
	};

	struct weight
	{
		int joint_idx;
		float bias;
		vec3 pos;
	};

	struct joint
	{
		string name;
		int parent;
		vec3 pos;
		quat orient;
	};

	struct mesh
	{
		string shader;
		vector<vertex> verts;
		vector<triangle> tris;  // TODO: vector<ivec3>
		vector<weight> weights;
		vector<vec3> positions, normals;
		vector<vec2> coords;
		vector<unsigned> indices;
	};

	bool prepare_mesh(mesh & m);
	bool prepare_mesh(mesh & m, md5animation::frame_skeleton const & s);
	bool prepare_normals(mesh & m);
	void render_mesh(mesh const & m);
	void render_normals(mesh const & m);
	void render_skeleton(vector<joint> const & joints);
	bool check_animation(md5animation const & anim) const;

	int _version, _joint_count, _mesh_count;
	vector<joint> _joints;
	vector<mesh> _meshes;
	mat4 _local_to_world;
	md5animation _anim;
	bool _has_anim;
};

md5animation::md5animation()
	: _version{0}
	, _frame_count{0}
	, _joint_count{0}
	, _animated_component_count{0}
	, _frame_rate{0}
	, _anim_duration{0}
	, _frame_duration{0}
	, _anim_time{0}
{}

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
		  q.w = -sqrtf(t);
}

bool md5animation::load_animation(std::string const & fname)
{
	if (!fs::exists(fname))
	{
		cerr << "unable to find animation file '" << fname << "'" << std::endl;
		return false;
	}

	fs::path fpath = fname;
	size_t flength = fs::file_size(fpath);

	ifstream in{fname};
	assert(in.is_open() && "unable to open animation file");

	_joint_infos.clear();
	_bounds.clear();
	_base_frames.clear();
	_frames.clear();
	_animated_skeleton.joints.clear();
	_frame_count = 0;

	string word, junk;

	in >> word;
	while (!in.eof())
	{
		if (word == "MD5Version")
		{
			in >> _version;
			assert(_version == 10);
		}
		else if (word == "commandline")
			in.ignore(flength, '\n');
		else if (word == "numFrames")
		{
			in >> _frame_count;
			in.ignore(flength, '\n');
		}
		else if (word == "numJoints")
		{
			in >> _joint_count;
			in.ignore(flength, '\n');
		}
		else if (word == "frameRate")
		{
			in >> _frame_rate;
			in.ignore(flength, '\n');
		}
		else if (word == "numAnimatedComponents")
		{
			in >> _animated_component_count;
			in.ignore(flength, '\n');
		}
		else if (word == "hierarchy")
		{
			in >> junk;  // '{'
			for (int i = 0; i < _joint_count; ++i)
			{
				joint_info j;
				in >> j.name >> j.parent >> j.flags >> j.start_index;
				remove_quotes(j.name);
				_joint_infos.push_back(j);
				in.ignore(flength, '\n');
			}
			in >> junk;  // '}'
		}
		else if (word == "bounds")
		{
			in >> junk;  // '{'
			in.ignore(flength, '\n');
			for (int i = 0; i < _frame_count; ++i)
			{
				bound b;
				in >> junk >>b.min_corner.x >> b.min_corner.y >> b.min_corner.z >> junk
					>> junk >> b.max_corner.x >> b.max_corner.y >> b.max_corner.z >> junk;
				_bounds.push_back(b);
				in.ignore(flength, '\n');
			}
			in >> junk;  // '}'
			in.ignore(flength, '\n');
		}
		else if (word == "baseframe")
		{
			in >> junk;  // '{'
			in.ignore(flength, '\n');
			for (int i = 0; i < _joint_count; ++i)
			{
				base_frame f;
				in >> junk >> f.pos.x >> f.pos.y >> f.pos.z >> junk
					>> junk >> f.orient.x >> f.orient.y >> f.orient.z >> junk;
				in.ignore(flength, '\n');
				_base_frames.push_back(f);
			}
			in >> junk;  // '}'
			in.ignore(flength, '\n');
		}
		else if (word == "frame")
		{
			frame_data f;
			in >> f.frame_idx >> junk;
			in.ignore(flength, '\n');

			f.data.reserve(_animated_component_count);
			for (int i = 0; i < _animated_component_count; ++i)
			{
				float d;
				in >> d;
				f.data.push_back(d);
			}

			_frames.push_back(f);
			build_frame_skeleton(f);

			in >> junk;  // '}'
			in.ignore(flength, '\n');
		}

		in >> word;
	}  // whiel (!in.eof

	_animated_skeleton.joints.assign(_joint_count, skeleton_joint{});

	_frame_duration = 1.0f/(float)_frame_rate;
	_anim_duration = (_frame_duration * (float)_frame_count);
	_anim_time = 0.0f;

	assert(_joint_infos.size() == _joint_count);
	assert(_bounds.size() == _frame_count);
	assert(_base_frames.size() == _joint_count);
	assert(_frames.size() == _frame_count);
	assert(_skeletons.size() == _frame_count);

	return true;
}

void md5animation::update(float dt)
{
	if (_frame_count < 1)
		return;

	_anim_time += dt;

	while (_anim_time > _anim_duration)
		_anim_time -= _anim_duration;

	while (_anim_time < 0)
		_anim_time += _anim_duration;

	// which frame we are on
	float frame_num = _anim_time * _frame_rate;
	int frame0 = (int)floorf(frame_num);
	int frame1 = (int)ceilf(frame_num);
	frame0 %= _frame_count;
	frame1 %= _frame_count;

	float interpolate = fmodf(_anim_time, _frame_duration) / _frame_duration;

	interpolate_skeletons(_skeletons[frame0], _skeletons[frame1], interpolate, _animated_skeleton);
}

void md5animation::render()
{
	glPointSize( 5.0f );
	glColor3f( 1.0f, 0.0f, 0.0f );

	glPushAttrib(GL_ENABLE_BIT);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	vector<skeleton_joint> const & joints = _animated_skeleton.joints;

	// Draw the joint positions
	glBegin(GL_POINTS);
	{
	  for (unsigned i = 0; i < joints.size(); ++i)
			glVertex3fv(glm::value_ptr(joints[i].pos));
	}
	glEnd();

	// Draw the bones
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);
	{
	  for (unsigned i = 0; i < joints.size(); ++i)
	  {
	      skeleton_joint const & j0 = joints[i];
			if (j0.parent != -1)
	      {
				skeleton_joint const & j1 = joints[j0.parent];
				glVertex3fv(glm::value_ptr(j0.pos));
				glVertex3fv(glm::value_ptr(j1.pos));
	      }
	  }
	}
	glEnd();

	glPopAttrib();
}

void md5animation::build_frame_skeleton(frame_data const & f)
{
	frame_skeleton s;
	for (unsigned i = 0; i < _joint_infos.size(); ++i)
	{
		unsigned k = 0;
		joint_info const & jinfo = _joint_infos[i];
		skeleton_joint janimated = _base_frames[i];

		janimated.parent = jinfo.parent;

		if (jinfo.flags & 1)  // pos.x
			janimated.pos.x = f.data[jinfo.start_index + k++];
		if (jinfo.flags & 2)  // pos.y
			janimated.pos.y = f.data[jinfo.start_index + k++];
		if (jinfo.flags & 4)  // pos.z
			janimated.pos.z = f.data[jinfo.start_index + k++];
		if (jinfo.flags & 8)  // orient.x
			janimated.orient.x = f.data[jinfo.start_index + k++];
		if (jinfo.flags & 16)  // orient.y
			janimated.orient.y = f.data[jinfo.start_index + k++];
		if (jinfo.flags & 32)  // orient.z
			janimated.orient.z = f.data[jinfo.start_index + k++];

		compute_quat_w(janimated.orient);

		if (janimated.parent >= 0)  // has a parent
		{
			skeleton_joint & parent = s.joints[janimated.parent];
			vec3 rotpos = parent.orient * janimated.pos;
			janimated.pos = parent.pos + rotpos;
			janimated.orient = parent.orient * janimated.orient;
			janimated.orient = normalize(janimated.orient);
		}

		s.joints.push_back(janimated);
	}

	_skeletons.push_back(s);
}

void md5animation::interpolate_skeletons(frame_skeleton const & a, frame_skeleton const & b, float interp, frame_skeleton & result)
{
	for (int i = 0; i < _joint_count; ++i)
	{
		skeleton_joint & j = result.joints[i];
		skeleton_joint const & j0 = a.joints[i];
		skeleton_joint const & j1 = b.joints[i];
		j.parent = j0.parent;
		j.pos = lerp(j0.pos, j1.pos, interp);  // TODO: preco nie slerp()
		j.orient = mix(j0.orient, j1.orient, interp);
	}
}

md5model::md5model()
	: _version{-1}
	, _joint_count{0}
	, _mesh_count{0}
	, _local_to_world{1}
	, _has_anim{false}
{}

bool md5model::load_model(string const & fname)
{
	if (!fs::exists(fname))
	{
		cerr << "unbleto load a model '" << fname << "'\n";
		return false;
	}

	fs::path fpath = fname;
	fs::path fparent_path = fpath.parent_path();

	size_t flength = fs::file_size(fpath);
	assert(flength > 0 && "empty model file");
	
	_joints.clear();
	_meshes.clear();

	string word, junk;

	ifstream in{fname};
	assert(in.is_open() && "unable to open a model file");

	in >> word;

	while (!in.eof())
	{
		if (word == "MD5Version")
		{
			in >> _version;
			assert(_version == 10 && "only version 10 (doom3) supported");
		}
		else if (word == "commandline")
			in.ignore(flength, '\n');
		else if (word == "numJoints")
		{
			in >> _joint_count;
			_joints.reserve(_joint_count);
		}
		else if (word == "numMeshes")
		{
			in >> _mesh_count;
			_meshes.reserve(_mesh_count);
		}
		else if (word == "joints")
		{
			joint j;
			in >> junk;  // '{'
			for (int i = 0; i < _joint_count; ++i)
			{
				in >> j.name >> j.parent 
					>> junk >> j.pos.x >> j.pos.y >> j.pos.z >> junk
					>> junk >> j.orient.x >> j.orient.y >> j.orient.z >> junk;

				remove_quotes(j.name);
				compute_quat_w(j.orient);

				_joints.push_back(j);

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

					// ignore shader ...

					in.ignore(flength, '\n');
				}
				else if (word == "numverts")
				{
					int numverts;
					in >> numverts;
					in.ignore(flength, '\n');
					for (int i = 0; i < numverts; ++i)
					{
						vertex v;
						in >> junk >> junk  // ignore vert, vertIndex
							>> junk >> v.tex0.x >> v.tex0.y >> junk
							>> v.start_weight >> v.weight_count;
						in.ignore(flength, '\n');;

						m.verts.push_back(v);
						m.coords.push_back(v.tex0);
					}
				}
				else if (word == "numtris")
				{
					int numtris;
					in >> numtris;
					in.ignore(flength, '\n');
					for (int i = 0; i < numtris; ++i)
					{
						triangle t;
						in >> junk >> junk >> t.v0 >> t.v1 >> t.v2;
						in.ignore(flength, '\n');

						m.tris.push_back(t);
						m.indices.push_back(t.v0);
						m.indices.push_back(t.v1);
						m.indices.push_back(t.v2);
					}
				}
				else if (word == "numweights")
				{
					int numweights;
					in >> numweights;
					in.ignore(flength, '\n');
					for (int i = 0; i < numweights; ++i)
					{
						weight w;
						in >> junk >> junk >> w.joint_idx >> w.bias 
							>> junk >> w.pos.x >> w.pos.y >> w.pos.z >> junk;
						in.ignore(flength, '\n');

						m.weights.push_back(w);
					}
				}
				else
					in.ignore(flength, '\n');

				in >> word;
			}

			prepare_mesh(m);
			prepare_normals(m);

			_meshes.push_back(m);
		}  // mesh

		in >> word;
	}

	assert(_meshes.size() == _mesh_count);
	assert(_joints.size() == _joint_count);

	return true;
}

bool md5model::load_animation(string const & fname)
{
	if (_anim.load_animation(fname))
		_has_anim = check_animation(_anim);
	return _has_anim;
}

void md5model::update(float dt)
{
	if (!_has_anim)
		return;

	_anim.update(dt);
	md5animation::frame_skeleton const & s = _anim.skeleton();

	for (mesh & m : _meshes)
		prepare_mesh(m, s);
}

bool md5model::prepare_mesh(mesh & m)  // TODO: navratova hodnota je zbytocna
{
	m.coords.clear();
	m.positions.clear();

	// compute vertex positions
	for (unsigned i = 0; i < m.verts.size(); ++i)
	{
		vertex & v = m.verts[i];
		v.pos = v.normal = vec3{0};
		
		// sum the position of the weights
		for (int j = 0; j < v.weight_count; ++j)
		{
			weight & w = m.weights[v.start_weight + j];
			joint & jnt = _joints[w.joint_idx];
			vec3 rotpos = jnt.orient * w.pos;  // convert the weight position from joint local space to object space
			v.pos += (jnt.pos + rotpos) * w.bias;
		}

		m.coords.push_back(v.tex0);
		m.positions.push_back(v.pos);
	}

	return true;
}

bool md5model::prepare_mesh(mesh & m, md5animation::frame_skeleton const & s)
{
	for (unsigned i = 0; i < m.verts.size(); ++i)
	{
		vertex const & v = m.verts[i];
		vec3 & pos = m.positions[i];
		vec3 & normal = m.normals[i];

		pos = vec3{0};
		normal = vec3{0};

		for (int k = 0; k < v.weight_count; ++k)
		{
			weight const & w = m.weights[v.start_weight + k];
			md5animation::skeleton_joint const & j = s.joints[w.joint_idx];
			vec3 rotpos = j.orient * w.pos;
			pos += (j.pos + rotpos) * w.bias;
			normal += (j.orient * v.normal) * w.bias;
		}
	}
	return true;
}

bool md5model::prepare_normals(mesh & m)  // TODO: navratova hodnota je zbytocna
{
	m.normals.clear();

	for (unsigned i = 0; i < m.tris.size(); ++i)
	{
		triangle & t = m.tris[i];
		vertex & v0 = m.verts[t.v0];
		vertex & v1 = m.verts[t.v1];
		vertex & v2 = m.verts[t.v2];
		vec3 n = cross(v2.pos - v0.pos, v1.pos - v0.pos);
		v0.normal += n;
		v1.normal += n;
		v2.normal += n;
	}

	for (unsigned i = 0; i < m.verts.size(); ++i)
	{
		vertex & v = m.verts[i];
		vec3 n = normalize(v.normal);
		m.normals.push_back(n);

		// put the bind-pose normal into joint-local space
		v.normal = vec3{0};
		for (int k = 0; k < v.weight_count; ++k)
		{
			weight const & w = m.weights[v.start_weight + k];
			joint const & j = _joints[w.joint_idx];
			v.normal += (n * j.orient) * w.bias;  // TODO: ?? prevratene nasobenie v tomto kontexte predstavuje inv(M)*v ?
		}
	}

	return true;
}

void md5model::render()
{
	glPushMatrix();
	glMultMatrixf(glm::value_ptr(_local_to_world));

	for (mesh & m : _meshes)
		render_mesh(m);

	_anim.render();

	for (mesh & m : _meshes)
		render_normals(m);

	glPopMatrix();
}

void md5model::render_mesh(mesh const & m)
{
	glColor3f(1.0f, 1.0f, 1.0f);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	// glBindTexture(GL_TEXTURE_2D, ...
	glVertexPointer(3, GL_FLOAT, 0, &(m.positions[0]));
	glNormalPointer(GL_FLOAT, 0, &(m.normals[0]));
	glTexCoordPointer(2, GL_FLOAT, 0, &(m.coords[0]));

	glDrawElements(GL_TRIANGLES, m.indices.size(), GL_UNSIGNED_INT, &(m.indices[0]));

	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glBindTexture(GL_TEXTURE_2D, 0);  // unbind textrue

	assert(glGetError() == GL_NO_ERROR);
}

void md5model::render_normals(mesh const & m)
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glColor3f(1.0f, 1.0f, 0.0f);

	glBegin(GL_LINES);
	{
		for (unsigned i = 0; i < m.positions.size(); ++i)
		{
			vec3 const & p0 = m.positions[i];
			vec3 p1 = p0 + 0.2f * m.normals[i];
			glVertex3fv(glm::value_ptr(p0));
			glVertex3fv(glm::value_ptr(p1));
		}
	}
	glEnd();

	glPopAttrib();

	assert(glGetError() == GL_NO_ERROR);
}

void md5model::render_skeleton(vector<joint> const & joints)
{
	glPointSize(5.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glPushAttrib(GL_ENABLE_BIT);

   glDisable(GL_LIGHTING);
   glDisable(GL_DEPTH_TEST);

   // joint positions
   glBegin(GL_POINTS);
   {
   	for (unsigned i = 0; i < joints.size(); ++i)
   		glVertex3fv(glm::value_ptr(joints[i].pos));
   }
   glEnd();

   // draw the bones
   glColor3f(0.0f, 1.0f, 0.0f);

   glBegin(GL_LINES);
   {
   	for (unsigned i = 0; i < joints.size(); ++i)
   	{
   		joint const & j0 = joints[i];
   		if (j0.parent != -1)
   		{
   			joint const & j1 = joints[j0.parent];
   			glVertex3fv(glm::value_ptr(j0.pos));
   			glVertex3fv(glm::value_ptr(j1.pos));
   		}
   	}
   }
   glEnd();

   glPopAttrib();

	assert(glGetError() == GL_NO_ERROR);
}

bool md5model::check_animation(md5animation const & anim) const
{
	if (_joint_count != anim.joint_count())
		return false;

	// joints check
	for (unsigned i = 0; i < _joints.size(); ++i)
	{
		joint const & j = _joints[i];
		md5animation::joint_info const & ji = anim.joint_info_at(i);
		if (j.name != ji.name || j.parent != ji.parent)
			return false;
	}

	return true;
}


GLfloat g_LighPos[] = { -39.749374, -6.182379, 46.334176, 1.0f };       // This is the position of the 'lamp' joint in the test mesh in object-local space
GLfloat g_LightAmbient[] = { 0.8f, 0.8f, 0.8f, 1.0f };
GLfloat g_LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat g_LightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat g_LighAttenuation0 = 1.0f;
GLfloat g_LighAttenuation1 = 0.0f;
GLfloat g_LighAttenuation2 = 0.0f;


class scene_window : public ui::glut_pool_window
{
public:
	using base = ui::glut_pool_window;

	scene_window();
	void display() override;
	void update(float dt) override;
	void input(float dt) override;
	void reshape(int w, int h) override;  // TODO: presun do okna

private:
	float _fovy = 45.0f, _near = 0.1f, _far = 1000.0f;

	md5model _model;
	gl::camera _cam;
	vector<shared_ptr<gl::camera_controller>> _ctrls;
};

void draw_axis(float fScale, glm::vec3 translate = glm::vec3(0));


scene_window::scene_window()
	: base{parameters{}.name("MD5 Model Loader")}
	, _cam{radians(_fovy), aspect_ratio(), _near, _far}	
{
	// gl setup
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClearDepth(1.0f);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, g_LightAmbient);

	glLightfv(GL_LIGHT0, GL_DIFFUSE, g_LightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, g_LightSpecular);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, g_LighAttenuation0);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, g_LighAttenuation1);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, g_LighAttenuation2);

	glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );

	assert(glGetError() == GL_NO_ERROR);

	// load model
	_model.load_model(model_path);
	_model.load_animation(anim_path);

	_cam.position = vec3{0,0,30};

	_ctrls.push_back(shared_ptr<gl::free_move<scene_window>>{new gl::free_move<scene_window>{_cam, *this, 0.4f}});
	_ctrls.push_back(shared_ptr<gl::free_look<scene_window>>{new gl::free_look<scene_window>{_cam, *this}});
}

void scene_window::display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	mat4 V = _cam.view();
	mat4 M = rotate(mat4{1}, radians(-90.0f), vec3{1,0,0});
	mat4 MV = V*M;
	glMultMatrixf(glm::value_ptr(MV));

	draw_axis(20.0f);

	glLightfv( GL_LIGHT0, GL_POSITION, g_LighPos );

	// Draw a debug sphere showing where the light is.positioned
	glTranslatef(g_LighPos[0], g_LighPos[1], g_LighPos[2]);
		glDisable(GL_LIGHTING);
		glColor3f(1.0f, 1.0f, 0.0f);
		glutWireSphere(1.0f, 8, 8);
		glEnable(GL_LIGHTING);
	glTranslatef(-g_LighPos[0], -g_LighPos[1], -g_LighPos[2]);

	assert(glGetError() == GL_NO_ERROR);

	_model.render();

	base::display();
}

void scene_window::update(float dt)
{
	_model.update(dt);
	base::update(dt);
}

void scene_window::input(float dt)
{
	for (auto ctrl : _ctrls)
		ctrl->input(dt);

	base::input(dt);
}

void scene_window::reshape(int w, int h)
{
	base::reshape(w, h);

	// set projection transformation
	glMatrixMode(GL_PROJECTION);
	gluPerspective(_fovy, aspect_ratio(), _near, _far);
}

void draw_axis(float fScale, glm::vec3 translate)
{
    glPushAttrib( GL_ENABLE_BIT );

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_LIGHTING );

    glPushMatrix();
    glTranslatef( translate.x, translate.y, translate.z );
    glScalef( fScale, fScale, fScale );
    glColor3f( 0.0f, 0.0f, 1.0f );

    glBegin( GL_LINES );
    {
        glColor3f( 1.0f, 0.0f, 0.0f );
        glVertex3f( 0.0f, 0.0f, 0.0 );
        glVertex3f( 1.0f, 0.0f, 0.0f );

        glColor3f( 0.0f, 1.0f, 0.0f );
        glVertex3f( 0.0f, 0.0f, 0.0f );
        glVertex3f( 0.0f, 1.0f, 0.0f );

        glColor3f( 0.0f, 0.0f, 1.0f );
        glVertex3f( 0.0f, 0.0f, 0.0f );
        glVertex3f( 0.0f, 0.0f, 1.0f );
    }
    glEnd();

    glPopMatrix();

    glPopAttrib();
}


int main(int argc, char * argv[])
{
	scene_window w;
	w.start();
	return 0;
}
