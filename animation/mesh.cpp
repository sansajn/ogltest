#include "mesh.hpp"
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <cassert>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GL/glew.h>

using std::fill_n;
using std::swap;
using std::vector;
using std::string;
using std::shared_ptr;
using std::ostringstream;
using std::runtime_error;

namespace gl {

mesh extract_mesh(aiMesh const & m);

attribute::attribute(unsigned index, int size, int type, unsigned stride, int start_idx, int normalized)
	: index{index}, size{size}, type{type}, normalized{normalized}, stride{stride}, start_idx{start_idx}
{
	switch (type)
	{
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
		case GL_INT:
		case GL_UNSIGNED_INT:
			int_type = true;
			break;

		default:
			int_type = false;
	}
}

mesh::mesh()
	: _nindices{0}, _draw_mode{GL_TRIANGLES}
{
	fill_n(_gpu_buffer_ids, buffer_count, 0);
}

mesh::mesh(void const * vbuf, unsigned vbuf_sizeof, unsigned const * ibuf, unsigned ibuf_size)
	: _nindices{ibuf_size}, _draw_mode{GL_TRIANGLES}
{
	fill_n(_gpu_buffer_ids, buffer_count, 0);

	glGenBuffers(buffer_count, _gpu_buffer_ids);
	glBindBuffer(GL_ARRAY_BUFFER, _gpu_buffer_ids[vbo_id]);
	glBufferData(GL_ARRAY_BUFFER, vbuf_sizeof, vbuf, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _gpu_buffer_ids[ibo_id]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ibuf_size*sizeof(unsigned), ibuf, GL_STATIC_DRAW);

	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

mesh::mesh(mesh && other)
{
	_nindices = other._nindices;
	swap(_attribs, other._attribs);
	_gpu_buffer_ids[vbo_id] = other._gpu_buffer_ids[vbo_id];
	_gpu_buffer_ids[ibo_id] = other._gpu_buffer_ids[ibo_id];
	other._gpu_buffer_ids[vbo_id] = other._gpu_buffer_ids[ibo_id] = 0;
	_draw_mode = other._draw_mode;
}

mesh::~mesh()
{
	glDeleteBuffers(buffer_count, _gpu_buffer_ids);
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void mesh::append_attribute(attribute const & a)
{
	_attribs.push_back(a);
}

void mesh::draw_mode(int mode)
{
	_draw_mode = mode;
}

void mesh::render() const
{
	glBindBuffer(GL_ARRAY_BUFFER, _gpu_buffer_ids[vbo_id]);  // vertices

	for (attribute const & a : _attribs)
	{
		glEnableVertexAttribArray(a.index);
		if (a.int_type)
			glVertexAttribIPointer(a.index, a.size, a.type, a.stride, (GLvoid *)(a.start_idx));
		else
			glVertexAttribPointer(a.index, a.size, a.type, a.normalized, a.stride, (GLvoid *)(a.start_idx));
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _gpu_buffer_ids[ibo_id]);  // indices

	glDrawElements(_draw_mode, _nindices, GL_UNSIGNED_INT, 0);

	for (attribute const & a : _attribs)
		glDisableVertexAttribArray(a.index);

	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void mesh::operator=(mesh && other)
{
	swap(_nindices, other._nindices);
	swap(_attribs, other._attribs);
	swap(_gpu_buffer_ids[vbo_id], other._gpu_buffer_ids[vbo_id]);
	swap(_gpu_buffer_ids[ibo_id], other._gpu_buffer_ids[ibo_id]);
	swap(_draw_mode, other._draw_mode);
}

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

mesh mesh_from_vertices(std::vector<vertex> const & verts, std::vector<unsigned> const & indices)
{
	vector<float> vbuf;
	vbuf.resize(verts.size()*(3+2+3+3));

	float * fptr = vbuf.data();
	for (vertex const & v : verts)
	{
		*fptr++ = v.position.x;
		*fptr++ = v.position.y;
		*fptr++ = v.position.z;
		*fptr++ = v.uv.x;
		*fptr++ = v.uv.y;
		*fptr++ = v.normal.x;
		*fptr++ = v.normal.y;
		*fptr++ = v.normal.z;
		*fptr++ = v.tangent.x;
		*fptr++ = v.tangent.y;
		*fptr++ = v.tangent.z;
	}

	mesh m(vbuf.data(), vbuf.size()*sizeof(float), indices.data(), indices.size());
	// TODO: vertex by mal poskytnut attributy
	unsigned stride = (3+2+3+3)*sizeof(GL_FLOAT);
	m.append_attribute(attribute{0, 3, GL_FLOAT, stride});  // position
	m.append_attribute(attribute{1, 2, GL_FLOAT, stride, 3*sizeof(GL_FLOAT)});  // texcoord
	m.append_attribute(attribute{2, 3, GL_FLOAT, stride, (3+2)*sizeof(GL_FLOAT)});  // normal
	m.append_attribute(attribute{3, 3, GL_FLOAT, stride, (3+2+3)*sizeof(GL_FLOAT)});  // tangent

	return m;
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

model::model(model && other)
	: _meshes{move(other._meshes)}
{}

void model::operator=(model && other)
{
	swap(_meshes, other._meshes);
}

model model_from_file(std::string const & fname)
{
	Assimp::Importer importer;
	aiScene const * scene = importer.ReadFile(fname,
		aiProcess_Triangulate|aiProcess_GenSmoothNormals|aiProcess_CalcTangentSpace|aiProcess_JoinIdenticalVertices);

	if (!scene)
		throw runtime_error{string{"assimp: "} + string{importer.GetErrorString()}};

	model m;
	for (int i = 0; i < scene->mNumMeshes; ++i)
		m.append_mesh(shared_ptr<mesh>{new mesh{extract_mesh(*scene->mMeshes[i])}});

	return m;
}

mesh make_quad_xy()
{
	return make_quad_xy(glm::vec2(-1,-1), 2.0f);
}

mesh make_unit_quad_xy()
{
	return make_quad_xy(glm::vec2{0,0}, 1);
}

mesh make_quad_xy(glm::vec2 const & origin, float size)
{
	std::vector<vertex> verts{
		{glm::vec3(origin, 0), glm::vec2(0,0), glm::vec3(0,0,1)},
		{glm::vec3(origin + glm::vec2(size, 0), 0), glm::vec2(1,0), glm::vec3(0,0,1)},
		{glm::vec3(origin + glm::vec2(size, size), 0), glm::vec2(1,1), glm::vec3(0,0,1)},
		{glm::vec3(origin + glm::vec2(0, size), 0), glm::vec2(0,1), glm::vec3(0,0,1)}
	};

	std::vector<unsigned> indices{0,1,2, 2,3,0};

	return mesh_from_vertices(verts, indices);
}

mesh make_quad_xz()
{
	return make_quad_xz(glm::vec2(-1,-1), 2.0f);
}

mesh make_quad_xz(glm::vec2 const & origin, float size)
{
	glm::vec2 const & o = origin;
	std::vector<vertex> verts{
		{glm::vec3(o.x, 0, o.y), glm::vec2(0,0), glm::vec3(0,1,0)},
		{glm::vec3(o.x + size, 0, o.y), glm::vec2(1,0), glm::vec3(0,1,0)},
		{glm::vec3(o.x + size, 0, o.y + size), glm::vec2(1,1), glm::vec3(0,1,0)},
		{glm::vec3(o.x, 0, o.y + size), glm::vec2(0,1), glm::vec3(0,1,0)}
	};

	std::vector<unsigned> indices{0,1,2, 2,3,0};

	return mesh_from_vertices(verts, indices);
}

mesh make_plane_xy(glm::vec3 const & origin, float size, unsigned w, unsigned h)
{
	assert(w > 1 && h > 1 && "invalid dimensions");

	// vertices
	float dx = 1.0f/(w-1);
	float dy = 1.0f/(h-1);
	std::vector<vertex> verts(w*h);

	for (int j = 0; j < h; ++j)
	{
		float py = j*dy;
		unsigned yoffset = j*w;
		for (int i = 0; i < w; ++i)
		{
			float px = i*dx;
			verts[i + yoffset] = vertex(glm::vec3(origin.x + size*px, origin.y + size*py, origin.z), glm::vec2(px, py), glm::vec3(0,0,1));
		}
	}

	// indices
	unsigned nindices = 2*(w-1)*(h-1)*3;
	std::vector<unsigned> indices(nindices);
	unsigned * indices_ptr = &indices[0];
	for (int j = 0; j < h-1; ++j)
	{
		unsigned yoffset = j*w;
		for (int i = 0; i < w-1; ++i)
		{
			int n = i + yoffset;
			*(indices_ptr++) = n;
			*(indices_ptr++) = n+1;
			*(indices_ptr++) = n+1+w;
			*(indices_ptr++) = n+1+w;
			*(indices_ptr++) = n+w;
			*(indices_ptr++) = n;
		}
	}

	return mesh_from_vertices(verts, indices);
}

mesh make_plane_xy(unsigned w, unsigned h)
{
	assert(w > 1 && h > 1 && "invalid dimensions");

	// vertices
	float dx = 1.0f/(w-1);
	float dy = 1.0f/(h-1);
	std::vector<vertex> verts(w*h);

	for (int j = 0; j < h; ++j)
	{
		float py = j*dy;
		unsigned yoffset = j*w;
		for (int i = 0; i < w; ++i)
		{
			float px = i*dx;
			verts[i + yoffset] = vertex(glm::vec3(px, py, 0), glm::vec2(px, py), glm::vec3(0,0,1));
		}
	}

	// indices
	unsigned nindices = 2*(w-1)*(h-1)*3;
	std::vector<unsigned> indices(nindices);
	unsigned * indices_ptr = &indices[0];
	for (int j = 0; j < h-1; ++j)
	{
		unsigned yoffset = j*w;
		for (int i = 0; i < w-1; ++i)
		{
			int n = i + yoffset;
			*(indices_ptr++) = n;
			*(indices_ptr++) = n+1;
			*(indices_ptr++) = n+1+w;
			*(indices_ptr++) = n+1+w;
			*(indices_ptr++) = n+w;
			*(indices_ptr++) = n;
		}
	}

	return mesh_from_vertices(verts, indices);
}

mesh make_plane_xz(unsigned w, unsigned h)
{
	assert(w > 1 && h > 1 && "invalid dimensions");

	// vertices
	float dx = 1.0f/(w-1);
	float dy = 1.0f/(h-1);
	std::vector<vertex> verts(w*h);

	for (int j = 0; j < h; ++j)
	{
		float pz = j*dy;
		unsigned yoffset = j*w;
		for (int i = 0; i < w; ++i)
		{
			float px = i*dx;
			verts[i + yoffset] = vertex(glm::vec3(px, 0, -pz), glm::vec2(px, pz), glm::vec3(0,1,0));
		}
	}

	// indices
	unsigned nindices = 2*(w-1)*(h-1)*3;
	std::vector<unsigned> indices(nindices);
	unsigned * indices_ptr = &indices[0];
	for (int j = 0; j < h-1; ++j)
	{
		unsigned yoffset = j*w;
		for (int i = 0; i < w-1; ++i)
		{
			int n = i + yoffset;
			*(indices_ptr++) = n;
			*(indices_ptr++) = n+1;
			*(indices_ptr++) = n+1+w;
			*(indices_ptr++) = n+1+w;
			*(indices_ptr++) = n+w;
			*(indices_ptr++) = n;
		}
	}

	return mesh_from_vertices(verts, indices);
}

mesh make_cube()
{
	static std::string const cube_desc{"hex 0 0 0 1"};
	return mesh_from_memory(cube_desc.c_str(), cube_desc.size(), "nff");
}

mesh make_cube(glm::vec3 const & position, float size)
{
	ostringstream oss;
	oss << "hex " << position.x << " " << position.y << " " << position.z << " " << size;
	string object_desc = oss.str();

	return mesh_from_memory(object_desc.c_str(), object_desc.size(), "nff");
}

mesh make_sphere()
{
	static std::string const sphere_desc{"s 0.0 0.0 0.0 1.0"};
	return mesh_from_memory(sphere_desc.c_str(), sphere_desc.size(), "nff");
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

}  // gl
