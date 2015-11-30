#include "model_loader.hpp"
#include <sstream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GL/glew.h>

using std::vector;
using std::string;
using std::shared_ptr;
using std::runtime_error;
using std::logic_error;
using std::ostringstream;
using glm::vec2;
using glm::vec3;

namespace gl {

mesh extract_mesh(aiMesh const & m);

// TODO: oddelit assimp od mesh.cpp
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

// TODO: oddelit assimp od mesh.cpp
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

model model_from_file(string const & fname)
{
	Assimp::Importer importer;
	aiScene const * scene = importer.ReadFile(fname,
		aiProcess_Triangulate|aiProcess_GenSmoothNormals|aiProcess_CalcTangentSpace|aiProcess_JoinIdenticalVertices);

	if (!scene)
		throw runtime_error{string{"assimp: "} + string{importer.GetErrorString()}};

	model mdl;
	for (int i = 0; i < scene->mNumMeshes; ++i)
	{
		shared_ptr<mesh> m{new mesh{extract_mesh(*scene->mMeshes[i])}};
		if (i < scene->mNumMaterials)
		{
			assert(scene->mNumMaterials == scene->mNumMeshes && "ocakavam texturu pre kazdu mriezku");
			aiString texture_id;
			scene->mMaterials[i]->Get(AI_MATKEY_NAME, texture_id);
			mdl.append_mesh(m, string{texture_id.C_Str()});
		}
		else
			mdl.append_mesh(shared_ptr<mesh>{new mesh{extract_mesh(*scene->mMeshes[i])}});
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
	result.attach_attributes({
		attribute{0, 3, GL_FLOAT, stride},  // position
		attribute{1, 2, GL_FLOAT, stride, 3*sizeof(GLfloat)},  // texcoord
		attribute{2, 3, GL_FLOAT, stride, (3+2)*sizeof(GLfloat)},  // normal
		attribute{3, 3, GL_FLOAT, stride, (3+2+3)*sizeof(GLfloat)}  // tangent
	});

	return result;
}

}  // gl
