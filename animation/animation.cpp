#include "animation.hpp"
#include <algorithm>
#include <utility>
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <GL/glew.h>
#include "md5loader.hpp"

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
using glm::mat4_cast;

namespace gl {

static vector<skeletal_animation::bone> create_skeleton(md5::animation const & anim, unsigned frame);
static shared_ptr<gl::mesh> create_animated_mesh(md5::model const & mdl, int mesh_idx);

skeletal_animation::skeletal_animation(string const & fname)
{
	from_file(fname);
}

void skeletal_animation::from_file(string const & fname)
{
	md5::animation a{fname};
	_skeletons.reserve(a.frames.size());
	for (int i = 0; i < a.frames.size(); ++i)
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
	return nearest_skeleton(frame);
}

std::vector<skeletal_animation::bone> skeletal_animation::nearest_skeleton(float frame) const
{
	return _skeletons[min((unsigned)(frame+0.5f), frame_count()-1)];
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
	// TODO: nebolo by toto vhodne presunut do animacie ?
	_anim_time += dt;
	if (_anim_time >= _anim.frame_count() / float(_anim.frame_rate()))  // repeat mode
		_anim_time = 0;

	float frame = _anim.frame_rate() * _anim_time;
	vector<skeletal_animation::bone> skel = _anim.skeleton(frame);

	assert(frame < _anim.frame_count() && "out of frames");
	assert(skel.size() == _inverse_bind_pose.size() && "bind pose not match");

	_curr_skel_transfs.clear();
	_curr_skel_transfs.reserve(skel.size());
	for (unsigned i = 0; i < skel.size(); ++i)
		_curr_skel_transfs.push_back(
			translate(mat4{1}, skel[i].position) * mat4_cast(skel[i].orientation) * _inverse_bind_pose[i]);
}

void animated_model::assign_animation(skeletal_animation && a)
{
	_anim = move(a);
}

void animated_model::assign_inverse_bind_pose(vector<mat4> && pose)
{
	_inverse_bind_pose = move(pose);
}

vector<mat4> const & animated_model::skeleton() const
{
	return _curr_skel_transfs;
}

skeletal_animation const & animated_model::animation() const
{
	return _anim;
}

// TODO: optimalizuj
shared_ptr<gl::mesh> create_animated_mesh(md5::model const & mdl, int mesh_idx)
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

animated_model animated_model_from_file(string const & mesh_file, string const & anim_file)
{
	animated_model result;

	md5::model mdl{mesh_file};
	for (int i = 0; i < mdl.meshes.size(); ++i)
		result.append_mesh(create_animated_mesh(mdl, i));

	vector<mat4> inverse_bind_pose;
	inverse_bind_pose.reserve(mdl.joints.size());

	for (md5::joint const & j : mdl.joints)  // compute inverse bind pose
	{
		inverse_bind_pose.push_back(
			inverse(translate(mat4{1}, j.position) * mat4_cast(j.orientation)));
	}

	result.assign_inverse_bind_pose(move(inverse_bind_pose));

	result.assign_animation(skeletal_animation{anim_file});

	return result;
}

}  // gl
