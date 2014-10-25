#include "assimp_loader.hpp"
#include <memory>
#include <vector>
#include <cstdint>
#include <cassert>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "render/gpubuffer.h"
#include "core/utils.hpp"

memory_stream & operator<<(memory_stream & s, aiVector3D const & v)
{
	s << v.x << v.y << v.z;
	return s;
}

memory_stream & operator<<(memory_stream & s, aiColor4D const & c)
{
	s << c.r << c.g << c.b << c.a;
	return s;
}

template <typename T>
static void copy_indices(aiMesh const & mesh, std::vector<T> & buf);

ptr<mesh_buffers> assimp_loader::load(std::string const & fname)
{
	Assimp::Importer importer;
	aiScene const * scene = importer.ReadFile(fname, aiProcess_Triangulate|aiProcess_JoinIdenticalVertices);
	if (!scene)
		throw std::exception();  // TODO: specify can't load mesh file

	assert(scene->mNumMeshes > 0 && "mesh file neobsahuje mriezku");

	// attributes position:0, normal:1, uv:2, color:3 (same as in a ork-mesh)
	int const position_id = 0;
	int const normal_id = 1;
	int const uv_id = 2;
	int const color_id = 3;

	aiMesh & mesh = *scene->mMeshes[0];
	assert(mesh.mNumVertices > 0 && "mriezka neobsahuje ziadne vrcholy");

	// vertex buffer layout: position:3, normal:3, uv:2, color:4
	int nverts = mesh.mNumVertices;
	bool has_normals = mesh.HasNormals();
	bool has_uvs = mesh.HasTextureCoords(0);
	bool has_colors = mesh.HasVertexColors(0);

	int vertex_size = sizeof(float) * (
		3 +                      // positions
		(has_normals ? 3 : 0) +  // normals
		(has_uvs ? 2 : 0) +      // uv
		(has_colors ? 4 : 0));   // colors

	int bufsize = nverts*vertex_size;

	memory_stream mout(bufsize);

	for (int i = 0; i < nverts; ++i)
	{
		mout << mesh.mVertices[i];
		if (has_normals)
			mout << mesh.mNormals[i];
		if (has_uvs)
		{
			aiVector3D & uv = mesh.mTextureCoords[0][i];
			mout << uv.x << uv.y;
		}
		if (has_colors)
			mout << mesh.mColors[0][i];
	}

	ptr<gpubuffer> vertexbuf{new gpubuffer};
	vertexbuf->data(bufsize, mout.buf(), buffer_usage::STATIC_DRAW);

	mout.close();

	// mesh
	ptr<mesh_buffers> m = make_ptr<mesh_buffers>();
	m->mode = mesh_mode::triangles;
	m->nvertices = nverts;

	m->append_attribute(  // position
		make_ptr<attribute_buffer>(position_id, 3, attribute_type::f32, vertexbuf, vertex_size));

	if (has_normals)
		m->append_attribute(
			make_ptr<attribute_buffer>(normal_id, 3, attribute_type::f32, vertexbuf, vertex_size, 3*sizeof(float)));

	if (has_uvs)
	{
		int uvs_offset = sizeof(float) * (3 + (has_normals ? 3 : 0));
		m->append_attribute(
			make_ptr<attribute_buffer>(uv_id, 2, attribute_type::f32, vertexbuf, vertex_size, uvs_offset));
	}

	if (has_colors)
	{
		int color_offset = sizeof(float) * (3 + (has_uvs ? 2 : 0) + (has_normals ? 3 : 0));
		m->append_attribute(
			make_ptr<attribute_buffer>(color_id, 4, attribute_type::f32, vertexbuf, vertex_size, color_offset));
	}

	// indices
	assert(mesh.HasFaces() && "no faces there");

	ptr<gpubuffer> indexbuf{new gpubuffer};

	int nindices = mesh.mNumFaces*3;  // model is already triangulated
	attribute_type attrtype = attribute_type::ui32;

	if (nindices < 256)
	{
		std::vector<uint8_t>	indices;
		copy_indices(mesh, indices);
		indexbuf->data(indices.size()*sizeof(uint8_t), &indices[0], buffer_usage::STATIC_DRAW);
		attrtype = attribute_type::ui8;
	}
	else if (nindices < 65536)
	{
		std::vector<uint16_t> indices;
		copy_indices(mesh, indices);
		indexbuf->data(indices.size()*sizeof(uint16_t), &indices[0], buffer_usage::STATIC_DRAW);
		attrtype = attribute_type::ui16;
	}
	else
	{
		std::vector<uint32_t> indices;
		copy_indices(mesh, indices);
		indexbuf->data(indices.size()*sizeof(uint32_t), &indices[0], buffer_usage::STATIC_DRAW);
		attrtype = attribute_type::ui32;
	}

	m->indices(make_ptr<attribute_buffer>(-1, 1, attrtype, indexbuf));
	m->nindices = nindices;

	return m;
}

template <typename T>
void copy_indices(aiMesh const & mesh, std::vector<T> & buf)
{
	buf.reserve(mesh.mNumFaces*3);
	for (int i = 0; i < mesh.mNumFaces; ++i)
	{
		aiFace & f = mesh.mFaces[i];
		buf.push_back(T(f.mIndices[0]));
		buf.push_back(T(f.mIndices[1]));
		buf.push_back(T(f.mIndices[2]));
	}
}
