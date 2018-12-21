#include "model_loader_gles2.hpp"
#include <sstream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GL/glew.h>
#include "gles2/texture_loader_gles2.hpp"

namespace gles2 {

using std::vector;
using std::string;
using std::shared_ptr;
using std::runtime_error;
using std::logic_error;
using std::ostringstream;
using glm::vec2;
using glm::vec3;

namespace fs = boost::filesystem;


mesh extract_mesh(aiMesh const & m);

mesh mesh_from_file(string const & fname, unsigned mesh_idx)
{
	Assimp::Importer importer;
	aiScene const * scene = importer.ReadFile(fname,
		aiProcess_Triangulate|aiProcess_GenSmoothNormals|aiProcess_CalcTangentSpace|aiProcess_JoinIdenticalVertices);

	if (!scene)
		throw runtime_error{string{"assimp: "} + string{importer.GetErrorString()}};

	assert(scene->mNumMeshes > mesh_idx && "mesh index out of range");

	return extract_mesh(*scene->mMeshes[mesh_idx]);
}

mesh mesh_from_memory(void const * buf, unsigned len, char const * file_format)
{
	Assimp::Importer importer;
	aiScene const * scene = importer.ReadFileFromMemory(buf, len,
		aiProcess_Triangulate|aiProcess_GenSmoothNormals|aiProcess_CalcTangentSpace|aiProcess_JoinIdenticalVertices,
		file_format);

	if (!scene)
		throw runtime_error{string{"assimp: "} + string{importer.GetErrorString()}};

	assert(scene->mNumMeshes > 0 && "mesh index out of range");

	return extract_mesh(*scene->mMeshes[0]);
}


model model_from_file(char const * fname, model_loader_parameters const & params)
{
	Assimp::Importer importer;
	aiScene const * scene = importer.ReadFile(fname,
		aiProcess_Triangulate|aiProcess_GenSmoothNormals|aiProcess_CalcTangentSpace|aiProcess_JoinIdenticalVertices);

	if (!scene)
		throw runtime_error{string{"assimp: "} + string{importer.GetErrorString()}};

	fs::path root_path{fname};
	root_path.remove_filename();

	model mdl;
	for (unsigned i = 0; i < scene->mNumMeshes; ++i)
	{
		shared_ptr<mesh> m{new mesh{extract_mesh(*scene->mMeshes[i])}};
		if (i < scene->mNumMaterials && !params.ignore_textures)
		{
			assert(scene->mNumMaterials == scene->mNumMeshes && "ocakavam texturu pre kazdu mriezku");
			aiString texture_name;  // vrati napr. bob_body
			scene->mMaterials[i]->Get(AI_MATKEY_NAME, texture_name);
			string tex_name = texture_name.C_Str();

			vector<property *> props = create_texture_mesh_properties(root_path.string(), tex_name, params);

			mdl.append_mesh(m, props);
		}
		else
			mdl.append_mesh(m);
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
	for (unsigned i = 0; i < m.mNumVertices; ++i)
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
	for (unsigned n = 0; n < m.mNumFaces; ++n)
	{
		aiFace & f = m.mFaces[n];
		*iptr++ = f.mIndices[0];
		*iptr++ = f.mIndices[1];
		*iptr++ = f.mIndices[2];
	}

	mesh result(vbuf.data(), vbuf.size()*sizeof(float), ibuf.data(), ibuf.size());
	unsigned stride = (3+2+3+3)*sizeof(GL_FLOAT);
	result.attach_attributes({
		attribute{0, 3, GL_FLOAT, stride},  // position
		attribute{1, 2, GL_FLOAT, stride, 3*sizeof(GLfloat)},  // texcoord
		attribute{2, 3, GL_FLOAT, stride, (3+2)*sizeof(GLfloat)},  // normal
		attribute{3, 3, GL_FLOAT, stride, (3+2+3)*sizeof(GLfloat)}  // tangent
	});

	return result;
}

vector<property *> create_texture_mesh_properties(string const & root, string const & tex_name, model_loader_parameters const & params)
{
	vector<property *> props;

	fs::path root_path{root};
	fs::path tex_filename = fs::path{tex_name}.filename();  // ignore texture sub-path

	// diffuse texture	
	fs::path tex_path = root_path / fs::path{tex_filename.string() + params.diffuse_texture_postfix + params.file_format};  // "<name>" or "<name>_d"
	if (!fs::exists(tex_path))
		throw std::logic_error{"diffuse texture '" + tex_path.string() + "' not found"};

	shared_ptr<texture2d> tex{new texture2d{texture_from_file(tex_path.c_str())}};
	props.push_back(new texture_property{tex, params.diffuse_uniform_name, params.diffuse_texture_bind_unit});

	// normal texture
	tex_path = root_path / fs::path{tex_filename.string() + params.normal_texture_postfix + params.file_format};  // "<name>_local"
	if (!params.ignore_normal_texture && fs::exists(tex_path))
	{
		shared_ptr<texture2d> norm_tex{new texture2d{texture_from_file(tex_path.c_str())}};
		props.push_back(new texture_property{norm_tex, params.normal_uniform_name, params.normal_texture_bind_unit});
	}

	// height texture
	tex_path = root_path / fs::path{tex_filename.string() + params.height_texture_postfix + params.file_format};  // "<name>_h"
	if (!params.ignore_height_texture && fs::exists(tex_path))
	{
		shared_ptr<texture2d> height_tex{new texture2d{texture_from_file(tex_path.c_str())}};
		props.push_back(new texture_property{height_tex, params.height_uniform_name, params.height_texture_bind_unit});
	}

	return props;
}

}  // gles2
