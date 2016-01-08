#include "model.hpp"
#include <algorithm>
#include <utility>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <glm/gtx/transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "gl/md5loader.hpp"

using std::swap;
using std::move;
using std::string;
using std::vector;
using std::shared_ptr;
using std::unique_ptr;
using std::runtime_error;
using std::min;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::ivec4;
using glm::mat4;
using glm::translate;
using gl::attribute;
using gl::mesh;
using gl::skeletal_animation;

namespace fs = boost::filesystem;

static mesh extract_mesh(aiMesh const & m);
static shared_ptr<mesh> create_animated_textured_mesh(md5::model const & mdl, int mesh_idx);

texture_manager texman;  //  global texture manager

shared_ptr<texture2d> texture_manager::find(string const & key)
{
	auto it = _textures.find(key);
	if (it != _textures.end())
		return it->second;

	auto result = _textures.insert(
		make_pair(key, shared_ptr<texture2d>{new texture2d{get_texture_filename(key)}}));

	return result.first->second;
}

string texture_manager::get_texture_filename(string const & key) const
{
	string::size_type pos = key.rfind('/');
	if (pos != string::npos && pos < key.size()-1)
		return _root_path + "/" + key.substr(pos+1);
	else
		return string{};
}

bool texture_manager::exists(string const & texture_name) const
{
	return fs::exists(fs::path{get_texture_filename(_root_path + texture_name)});
}

void textured_model::append_mesh(std::shared_ptr<mesh> m, std::string const & texture_id, int texture_compound)
{
	_meshes.push_back(m);
	_texture_ids.push_back(texture_id);
	_texture_compound.push_back(texture_compound);
}

textured_model::textured_model(textured_model && other)
	: _texture_ids{move(other._texture_ids)}
	, _texture_compound{move(other._texture_compound)}
	, _meshes{move(other._meshes)}
{}

void textured_model::operator=(textured_model && other)
{
	swap(_texture_ids, other._texture_ids);
	swap(_texture_compound, other._texture_compound);
	swap(_meshes, other._meshes);
}

void textured_model::render(shader::program & prog)
{
	for (int i = 0; i < _meshes.size(); ++i)
	{
		string const & texid = _texture_ids[i];

		if (_texture_compound[i] & diffuse_texture)
		{
			shared_ptr<texture2d> diff_tex = texman.find(texid + "_d.tga");
			assert(diff_tex && "invalid texture pointer");
			diff_tex->bind(0);
			prog.uniform_variable("diff_tex", 0);
		}

		if (_texture_compound[i] & normal_texture)
		{
			shared_ptr<texture2d> norm_tex = texman.find(texid + "_local.tga");
			assert(norm_tex && "invalid texture pointer");
			norm_tex->bind(1);
			prog.uniform_variable("norm_tex", 1);
		}

		// TODO: ak nemama normal-mapu a shader snou pocita, potrebujem dat defaultnu verziu s hodnotami [0,0,1]

		if (_texture_compound[i] & height_texture)
		{
			shared_ptr<texture2d> height_tex = texman.find(texid + "_h.tga");
			assert(height_tex && "invalid texture pointer");
			height_tex->bind(2);
			prog.uniform_variable("height_tex", 2);
		}

		_meshes[i]->render();
	}
}

inline bool diffuse_exists(char const * path)
{
	return texman.exists(string{path} + "_d.tga");
}

inline bool local_normal_exists(char const * path)
{
	return texman.exists(string{path} + "_local.tga");
}

inline bool height_exists(char const * path)
{
	return texman.exists(string{path} + "_h.tga");
}

inline bool specular_exists(char const * path)
{
	return texman.exists(string{path} + "_s.tga");
}

inline bool glow_exists(char const * path)
{
	return texman.exists(string{path} + "_glow.tga");
}

textured_model textured_model_from_file(string const & fname)
{
	Assimp::Importer importer;
	aiScene const * scene = importer.ReadFile(fname,
		aiProcess_Triangulate|aiProcess_GenSmoothNormals|aiProcess_CalcTangentSpace|aiProcess_JoinIdenticalVertices);

	if (!scene)
		throw runtime_error{string{"assimp: "} + string{importer.GetErrorString()}};

	assert(scene->mNumMaterials == scene->mNumMeshes && "ocakavam texturu pre kazdu mriezku");

	textured_model mdl;
	for (int i = 0; i < scene->mNumMeshes; ++i)
	{
		aiString texture_id;
		scene->mMaterials[i]->Get(AI_MATKEY_NAME, texture_id);

		int type = textured_model::none_texture;
		if (diffuse_exists(texture_id.C_Str()))
			type |= textured_model::diffuse_texture;
		if (local_normal_exists(texture_id.C_Str()))
			type |= textured_model::normal_texture;
		if (height_exists(texture_id.C_Str()))
			type |= textured_model::height_texture;
		if (specular_exists(texture_id.C_Str()))
			type |= textured_model::specular_texture;
		if (glow_exists(texture_id.C_Str()))
			type |= textured_model::glow_texture;

		mdl.append_mesh(shared_ptr<mesh>{new mesh{extract_mesh(*scene->mMeshes[i])}}, string{texture_id.C_Str()}, type);
	}

	return mdl;
}

mesh extract_mesh(aiMesh const & m)
{
	// vertices
	vector<float> vbuf;
	unsigned vbuf_size = m.mNumVertices * (3+2+3+3);  // position, uv, normal, tangent
	vbuf.resize(vbuf_size);

	float * vptr = vbuf.data();
	for (int i = 0; i < m.mNumVertices; ++i)
	{
		aiVector3D & v = m.mVertices[i];
		*vptr++ = v.x;
		*vptr++ = v.y;
		*vptr++ = v.z;

		if (m.mTextureCoords[0])
		{
			aiVector3D & uv = m.mTextureCoords[0][i];
			*vptr++ = uv.x;
			*vptr++ = uv.y;
		}
		else  // texture uv not available
		{
			*vptr++ = 0.0f;
			*vptr++ = 0.0f;
		}

		aiVector3D & n = m.mNormals[i];
		*vptr++ = n.x;
		*vptr++ = n.y;
		*vptr++ = n.z;

		if (m.mTangents)
		{
			aiVector3D & t = m.mTangents[i];
			*vptr++ = t.x;
			*vptr++ = t.y;
			*vptr++ = t.z;
		}
		else  // tangents not available
		{
			*vptr++ = 0.0f;
			*vptr++ = 0.0f;
			*vptr++ = 0.0f;
		}
	}  // for (n

	// indices
	vector<unsigned> ibuf;
	unsigned ibuf_size = m.mNumFaces*3;  // predpoklada triangulaciu mriezky
	ibuf.resize(ibuf_size);

	unsigned * iptr = ibuf.data();
	for (int n = 0; n < m.mNumFaces; ++n)
	{
		aiFace & f = m.mFaces[n];
		*iptr++ = f.mIndices[0];
		*iptr++ = f.mIndices[1];
		*iptr++ = f.mIndices[2];
	}

	mesh result(vbuf.data(), vbuf.size()*sizeof(float), ibuf.data(), ibuf.size());
	unsigned stride = (3+2+3+3)*sizeof(GL_FLOAT);
	result.append_attribute(attribute{0, 3, GL_FLOAT, stride});  // position
	result.append_attribute(attribute{1, 2, GL_FLOAT, stride, 3*sizeof(GL_FLOAT)});  // texcoord
	result.append_attribute(attribute{2, 3, GL_FLOAT, stride, (3+2)*sizeof(GL_FLOAT)});  // normal
	result.append_attribute(attribute{3, 3, GL_FLOAT, stride, (3+2+3)*sizeof(GL_FLOAT)});  // tangent

	return result;
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

inline bool animated_textured_model::has_animation() const
{
	return !_anims.empty();
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
shared_ptr<mesh> create_animated_textured_mesh(md5::model const & mdl, int mesh_idx)
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
