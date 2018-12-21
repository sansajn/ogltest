#include "animation_gles2.hpp"
#include <algorithm>
#include <utility>
#include <cassert>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <GL/glew.h>
#include "gl/md5loader.hpp"
#include "gles2/model_loader_gles2.hpp"
#include "gles2/texture_loader_gles2.hpp"

namespace gles2 {

using std::move;
using std::unique_ptr;
using std::shared_ptr;
using std::vector;
using std::string;
using std::min;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::ivec4;
using glm::mat4;
using glm::quat;
using glm::cross;
using glm::normalize;
using glm::translate;
using glm::rotate;
using glm::inverse;
using glm::lerp;
using glm::slerp;
using glm::mix;
using glm::mat4_cast;

namespace fs = boost::filesystem;

static vector<skeletal_animation::bone> create_skeleton(md5::animation const & anim, unsigned frame);
static shared_ptr<mesh> create_animated_mesh(md5::model const & mdl, int mesh_idx);

skeletal_animation::skeletal_animation(string const & fname)
{
	from_file(fname);
}

void skeletal_animation::from_file(string const & fname)
{
	md5::animation a{fname};
	_skeletons.reserve(a.frames.size());
	for (unsigned i = 0; i < a.frames.size(); ++i)
		_skeletons.push_back(create_skeleton(a, i));

	_frame_rate = a.frame_rate;
}

unsigned skeletal_animation::frame_count() const
{
	return _skeletons.size();
}

unsigned skeletal_animation::frame_rate() const
{
	return _frame_rate;
}

vector<skeletal_animation::bone> const & skeletal_animation::operator[](int n) const
{
	return _skeletons[n];
}

vector<skeletal_animation::bone> skeletal_animation::skeleton(float frame) const
{
	return slerp_skeleton(frame);
}

std::vector<skeletal_animation::bone> skeletal_animation::nearest_skeleton(float frame) const
{
	return _skeletons[min((unsigned)(frame+0.5f), frame_count()-1)];
}

std::vector<skeletal_animation::bone> skeletal_animation::lerp_skeleton(float frame) const
{
	std::vector<skeletal_animation::bone> const & skel0 = _skeletons[(unsigned)frame];
	std::vector<skeletal_animation::bone> const & skel1 = _skeletons[min((unsigned)(frame+1), frame_count()-1)];
	std::vector<skeletal_animation::bone> result(skel0.size());

	float t = frame - int(frame);

	for (unsigned i = 0; i < skel0.size(); ++i)
	{
		bone & b = result[i];
		bone const & b0 = skel0[i];
		bone const & b1 = skel1[i];
		b.position = mix(b0.position, b1.position, t);
		b.orientation = lerp(b0.orientation, b1.orientation, t);
	}

	return result;
}

std::vector<skeletal_animation::bone> skeletal_animation::slerp_skeleton(float frame) const
{
	std::vector<skeletal_animation::bone> const & skel0 = _skeletons[(unsigned)frame];
	std::vector<skeletal_animation::bone> const & skel1 = _skeletons[min((unsigned)(frame+1), frame_count()-1)];
	std::vector<skeletal_animation::bone> result(skel0.size());

	float t = frame - int(frame);

	for (unsigned i = 0; i < skel0.size(); ++i)
	{
		bone & b = result[i];
		bone const & b0 = skel0[i];
		bone const & b1 = skel1[i];
		b.position = mix(b0.position, b1.position, t);
		b.orientation = slerp(b0.orientation, b1.orientation, t);
	}

	return result;
}

vector<skeletal_animation::bone> create_skeleton(md5::animation const & anim, unsigned frame)
{
	vector<skeletal_animation::bone> result(anim.hierarchy.size());

	vector<float> const & frame_data = anim.frames[frame];

	for (unsigned i = 0; i < anim.hierarchy.size(); ++i)
	{
		md5::hierarchy_record const & h = anim.hierarchy[i];

		skeletal_animation::bone & b = result[i];
		b.position = anim.base_frame[i].position;
		b.orientation = anim.base_frame[i].orientation;

		int idx = h.start_idx;
		if (h.flags & 1)
			b.position.x = frame_data[idx++];
		if (h.flags & 2)
			b.position.y = frame_data[idx++];
		if (h.flags & 4)
			b.position.z = frame_data[idx++];
		if (h.flags & 8)
			b.orientation.x = frame_data[idx++];
		if (h.flags & 16)
			b.orientation.y = frame_data[idx++];
		if (h.flags & 32)
			b.orientation.z = frame_data[idx++];

		md5::compute_quat_w(b.orientation);

		if (h.parent > -1)
		{
			assert(h.parent < result.size());
			assert(h.parent < i && "parent was not yet evaluated");

			skeletal_animation::bone const & b_parent = result[h.parent];
			b.position = b_parent.position + (b_parent.orientation * b.position);
			b.orientation = normalize(b_parent.orientation * b.orientation);
		}
	}

	return result;
}


void animated_model::update(float dt)
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

void animated_model::compute_skeleton_transformations(vector<skeletal_animation::bone> const & skel)
{
	_curr_skel_transfs.clear();
	_curr_skel_transfs.reserve(skel.size());
	for (unsigned i = 0; i < skel.size(); ++i)
	{
		_curr_skel_transfs.push_back(
			translate(mat4{1}, skel[i].position) * mat4_cast(skel[i].orientation) * _inverse_bind_pose[i]);
	}
}

void animated_model::animation_sequence(vector<unsigned> const & s, repeat_mode m)
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

animated_model::state animated_model::animation_state() const
{
	return _anim_state;
}

void animated_model::append_animation(skeletal_animation && a)
{
	_anims.push_back(move(a));
}

void animated_model::assign_inverse_bind_pose(vector<mat4> && pose)
{
	_inverse_bind_pose = move(pose);
}

vector<mat4> const & animated_model::skeleton() const
{
	if (_anim_seq.empty() && _curr_skel_transfs.empty())
		_curr_skel_transfs.assign(100, mat4{1});  // TODO: 100 mam zadratovane v shadery

	return _curr_skel_transfs;
}

skeletal_animation const & animated_model::animation() const
{
	return _anims[_anim_seq[_anim_seq_idx]];
}

bool animated_model::has_animation() const
{
	return !_anims.empty();
}


animated_model animated_model_from_file(std::string const & model_file, model_loader_parameters const & params)
{
	animated_model result;

	md5::model mdl{model_file};
	for (unsigned i = 0; i < mdl.meshes.size(); ++i)
	{
		fs::path root_path{model_file};
		root_path.remove_filename();
		string const & tex_name = mdl.meshes[i].shader;

		vector<property *> props;
		if (!params.ignore_textures)
			props = create_texture_mesh_properties(root_path.string(), tex_name, params);

		result.append_mesh(create_animated_mesh(mdl, i), props);
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

shared_ptr<mesh> create_animated_mesh(md5::model const & mdl, int mesh_idx)  // TODO: optimalizuj
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

	for (unsigned i = 0; i < m.indices.size(); i += 3)
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

	// tangents
	vector<vec3> tangents{positions.size()};

	for (unsigned i = 0; i < m.indices.size(); i += 3)
	{
		unsigned i0 = m.indices[i];
		unsigned i1 = m.indices[i+1];
		unsigned i2 = m.indices[i+2];

		vec3 & p0 = positions[i0];
		vec3 & p1 = positions[i1];
		vec3 & p2 = positions[i2];

		vec2 const & w0 = m.vertices[i0].uv;
		vec2 const & w1 = m.vertices[i1].uv;
		vec2 const & w2 = m.vertices[i2].uv;

		vec3 q = p1 - p0;
		vec3 r = p2 - p0;

		vec2 s = w1 - w0;
		vec2 t = w2 - w0;

		float k = 1.0f / (s.x*t.y - t.x*s.y);
		vec3 t_u = (t.y*q - s.y*r) * k;  // tangent vector v smere u

		tangents[i0] += t_u;
		tangents[i1] += t_u;
		tangents[i2] += t_u;
	}

	for (unsigned i = 0; i < tangents.size(); ++i)
	{
		vec3 & t = tangents[i];
		vec3 const & n = normals[i];
		t = normalize(t - n*dot(n,t));  // gram-schmidt ortogonalize and normalize
	}

	// joint indices (four per vertex)
	vector<ivec4> joints;
	for (md5::vertex const & v : m.vertices)
	{
//		assert(v.wcount < 5 && "only 4 weights per vertex supported");
		// TODO: model vykazuje artefakty, treba ohandlovat situacie v ktorych je pozet spojou > 4

		ivec4 ids{0};
		for (int i = 0; i < min(v.wcount, 4); ++i)
			ids[i] = m.weights[v.wid + i].jid;

		joints.push_back(ids);
	}

	// weigths (four per vertex)
	vector<vec4> weights;
	for (md5::vertex const & v : m.vertices)
	{
//		assert(v.wcount < 5 && "only 4 weights per vertex supported");

		vec4 influence{0};
		for (int i = 0; i < min(v.wcount, 4); ++i)
			influence[i] = m.weights[v.wid + i].influence;

		weights.push_back(influence);
	}

	// vytvor mesh buffer
	unsigned vertex_size = (3+2+3+3+4)*sizeof(float) + 4*sizeof(int);  // position:3, uv:2, normal:3, tangent:3, joint:4, weight:4
	unique_ptr<uint8_t[]> vbuf{new uint8_t[vertex_size*positions.size()]};
	float * fbuf = (float *)vbuf.get();

	for (unsigned i = 0; i < positions.size(); ++i)
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

		vec3 & t = tangents[i];
		*fbuf++ = t.x;
		*fbuf++ = t.y;
		*fbuf++ = t.z;

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

	shared_ptr<mesh> result{
		new mesh(vbuf.get(), vertex_size*positions.size(), m.indices.data(), m.indices.size())};

	unsigned stride = (3+2+3+3+4)*sizeof(GLfloat) + 4*sizeof(GLint);
	result->attach_attributes({
		mesh::vertex_attribute_type{0, 3, GL_FLOAT, stride},  // position
		mesh::vertex_attribute_type{1, 2, GL_FLOAT, stride, 3*sizeof(GLfloat)},  // texcoord
		mesh::vertex_attribute_type{2, 3, GL_FLOAT, stride, (3+2)*sizeof(GLfloat)},  // normal
		mesh::vertex_attribute_type{3, 3, GL_FLOAT, stride, (3+2+3)*sizeof(GLfloat)},  // tangent
		mesh::vertex_attribute_type{4, 3, GL_INT, stride, (3+2+3+3)*sizeof(GLfloat)},  // joints
		mesh::vertex_attribute_type{5, 4, GL_FLOAT, stride, (3+2+3+3)*sizeof(GLfloat) + 4*sizeof(GLint)}  // weights
	});

	assert(stride == vertex_size && "type michmach");

	return result;
}

}  // gles2
