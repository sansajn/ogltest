#include "model.hpp"
#include <algorithm>
#include <utility>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using std::swap;
using std::move;
using std::string;
using std::vector;
using std::shared_ptr;
using std::runtime_error;
using gl::attribute;

namespace fs = boost::filesystem;

static mesh extract_mesh(aiMesh const & m);

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

static bool diffuse_exists(char const * path)
{
	return texman.exists(string{path} + "_d.tga");
}

static bool local_normal_exists(char const * path)
{
	return texman.exists(string{path} + "_local.tga");
}

static bool height_exists(char const * path)
{
	return texman.exists(string{path} + "_h.tga");
}

static bool specular_exists(char const * path)
{
	return texman.exists(string{path} + "_s.tga");
}

static bool glow_exists(char const * path)
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
