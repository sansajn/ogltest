#include "mesh.hpp"
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <cassert>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GL/glew.h>

using std::swap;
using std::move;
using std::vector;
using std::string;
using std::shared_ptr;
using std::ostringstream;
using std::runtime_error;
using std::logic_error;
using glm::vec2;
using glm::vec3;

namespace gl {

mesh extract_mesh(aiMesh const & m);
GLenum opengl_cast(buffer_usage u);
GLenum opengl_cast(buffer_target t);
GLenum opengl_cast(render_primitive p);

gpu_buffer::gpu_buffer(unsigned size, buffer_usage usage)
{
	glGenBuffers(1, &_id);
	glBindBuffer(GL_COPY_WRITE_BUFFER, _id);
	glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, opengl_cast(usage));
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);  // unbind
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

gpu_buffer::gpu_buffer(void const * buf, unsigned size, buffer_usage usage)
{
	glGenBuffers(1, &_id);
	glBindBuffer(GL_COPY_WRITE_BUFFER, _id);
	glBufferData(GL_COPY_WRITE_BUFFER, size, buf, opengl_cast(usage));
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);  // unbind
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

gpu_buffer::gpu_buffer(gpu_buffer && other)
	: _id{other._id}
{
	other._id = 0;
}

void gpu_buffer::operator=(gpu_buffer && other)
{
	swap(_id, other._id);
}

gpu_buffer::~gpu_buffer()
{
	if (_id)
		glDeleteBuffers(1, &_id);
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void gpu_buffer::data(void const * buf, unsigned size, unsigned offset)
{
	assert(_id && "uninitialized buffer");
	glBindBuffer(GL_COPY_WRITE_BUFFER, _id);
	glBufferSubData(GL_COPY_WRITE_BUFFER, offset, size, buf);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

unsigned gpu_buffer::id() const
{
	return _id;
}

void gpu_buffer::bind(buffer_target target) const
{
	assert(_id && "uninitialized buffer");
	glBindBuffer(opengl_cast(target), _id);
	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

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

mesh::mesh(unsigned vbuf_size_in_bytes, unsigned index_count, buffer_usage usage)
	: _vbuf{vbuf_size_in_bytes, usage}, _ibuf(index_count*sizeof(unsigned), usage), _nindices{index_count}, _draw_mode{GL_TRIANGLES}
{}

mesh::mesh(void const * vbuf, unsigned vbuf_size, unsigned const * ibuf, unsigned ibuf_size, buffer_usage usage)
	: _vbuf{vbuf, vbuf_size, usage}, _ibuf(ibuf, ibuf_size*sizeof(unsigned), usage), _nindices{ibuf_size}, _draw_mode{GL_TRIANGLES}
{}

mesh::mesh(mesh && other)
	: _vbuf{move(other._vbuf)}
	, _ibuf{move(other._ibuf)}
	, _nindices{other._nindices}
	, _draw_mode{other._draw_mode}
{
	swap(_attribs, other._attribs);
}

void mesh::operator=(mesh && other)
{
	swap(_nindices, other._nindices);
	swap(_attribs, other._attribs);
	_vbuf = move(other._vbuf);
	_ibuf = move(other._ibuf);
	swap(_draw_mode, other._draw_mode);
}

void mesh::render() const
{
	_vbuf.bind(buffer_target::array);

	for (attribute const & a : _attribs)
	{
		glEnableVertexAttribArray(a.index);
		if (a.int_type)
			glVertexAttribIPointer(a.index, a.size, a.type, a.stride, (GLvoid *)(intptr_t)a.start_idx);
		else
			glVertexAttribPointer(a.index, a.size, a.type, a.normalized, a.stride, (GLvoid *)(intptr_t)a.start_idx);
	}

	_ibuf.bind(buffer_target::element_array);

	glDrawElements(_draw_mode, _nindices, GL_UNSIGNED_INT, 0);

	for (attribute const & a : _attribs)
		glDisableVertexAttribArray(a.index);

	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void mesh::append_attribute(attribute const & a)
{
	_attribs.push_back(a);
}

void mesh::draw_mode(render_primitive mode)
{
	_draw_mode = opengl_cast(mode);
}


void mesh::data(void const * vsubbuf, unsigned size, unsigned offset)
{
	_vbuf.data(vsubbuf, size, offset);
}

void mesh::data(void const * vbuf, unsigned vbuf_size, unsigned const * ibuf, unsigned ibuf_size)
{
	data(vbuf, vbuf_size, 0, ibuf, ibuf_size, 0);
}

void mesh::data(void const * vsubbuf, unsigned vsubbuf_size, unsigned vsubbuf_offset, unsigned const * isubbuf, unsigned isubbuf_size, unsigned isubbuf_offset)
{
	_vbuf.data(vsubbuf, vsubbuf_size, vsubbuf_offset);
	_ibuf.data(isubbuf, isubbuf_size*sizeof(unsigned), isubbuf_offset);
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
	// TODO: tu potrebujem pre kazdy mesh bindnut textury
	for (shared_ptr<mesh> m : _meshes)
		m->render();
}

void model::append_mesh(shared_ptr<mesh> m)
{
	_meshes.push_back(m);
}

void model::append_mesh(std::shared_ptr<mesh> m, string const & texture_id)
{
	_meshes.push_back(m);
	_texture_ids.push_back(texture_id);
}

model::model(model && other)
	: _texture_ids{move(other._texture_ids)}, _meshes{move(other._meshes)}
{}

void model::operator=(model && other)
{
	swap(_meshes, other._meshes);
	swap(_texture_ids, other._texture_ids);
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
		{glm::vec3(o.x, 0, -o.y), glm::vec2(0,0), glm::vec3(0,1,0)},
		{glm::vec3(o.x + size, 0, -o.y), glm::vec2(1,0), glm::vec3(0,1,0)},
		{glm::vec3(o.x + size, 0, -(o.y + size)), glm::vec2(1,1), glm::vec3(0,1,0)},
		{glm::vec3(o.x, 0, -(o.y + size)), glm::vec2(0,1), glm::vec3(0,1,0)}
	};

	std::vector<unsigned> indices{0,1,2, 2,3,0};

	return mesh_from_vertices(verts, indices);
}

mesh make_quad_zy()
{
	return make_quad_zy(vec2{-1,-1}, 2.0f);
}

mesh make_quad_zy(vec2 const & origin, float size)
{
	vec2 const & o = origin;

	vector<vertex> verts{
		{vec3{0, o.y, o.x}, vec2{0,0}, vec3{1,0,0}},
		{vec3{0, o.y, o.x + size}, vec2{1,0}, vec3{1,0,0}},
		{vec3{0, o.y + size, o.x + size}, vec2{1,1}, vec3{1,0,0}},
		{vec3{0, o.y + size, o.x}, vec2{0,1}, vec3{1,0,0}}
	};

	vector<unsigned> indices{0,1,2, 2,3,0};

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

mesh make_axis()
{
	vector<float> vertices{  // position, color
		0,0,0, 1,0,0,
		1,0,0, 1,0,0,
		0,0,0, 0,1,0,
		0,1,0, 0,1,0,
		0,0,0, 0,0,1,
		0,0,1, 0,0,1};

	vector<unsigned> indices{0,1, 2,3, 4,5};

	mesh m(vertices.data(), vertices.size()*sizeof(float), indices.data(), indices.size());
	m.append_attribute(attribute{0, 3, GL_FLOAT, 6*sizeof(GLfloat)});  // position
	m.append_attribute(attribute{1, 3, GL_FLOAT, 6*sizeof(GLfloat), 3*sizeof(GLfloat)});  // color
	m.draw_mode(render_primitive::lines);
	return m;
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

GLenum opengl_cast(buffer_usage u)
{
	switch (u)
	{
		case buffer_usage::stream_draw: return GL_STREAM_DRAW;
		case buffer_usage::stream_read: return GL_STREAM_READ;
		case buffer_usage::stream_copy: return GL_STREAM_COPY;
		case buffer_usage::static_draw: return GL_STATIC_DRAW;
		case buffer_usage::static_read: return GL_STATIC_READ;
		case buffer_usage::static_copy: return GL_STATIC_COPY;
		case buffer_usage::dynamic_draw: return GL_DYNAMIC_DRAW;
		case buffer_usage::dynamic_read: return GL_DYNAMIC_READ;
		case buffer_usage::dynamic_copy: return GL_DYNAMIC_COPY;
		default:
			throw logic_error{"bad cast, unknown buffer usage"};
	}
}

GLenum opengl_cast(buffer_target t)
{
	switch (t)
	{
		case buffer_target::array: return GL_ARRAY_BUFFER;
		case buffer_target::atomic_counter: return GL_ATOMIC_COUNTER_BUFFER;
		case buffer_target::copy_read: return GL_COPY_READ_BUFFER;
		case buffer_target::copy_write: return GL_COPY_WRITE_BUFFER;
		case buffer_target::draw_indirect: return GL_DRAW_INDIRECT_BUFFER;
		case buffer_target::dispatch_indirect: return GL_DISPATCH_INDIRECT_BUFFER;
		case buffer_target::element_array: return GL_ELEMENT_ARRAY_BUFFER;
		case buffer_target::pixel_pack: return GL_PIXEL_PACK_BUFFER;
		case buffer_target::pixel_unpack: return GL_PIXEL_UNPACK_BUFFER;
		case buffer_target::query: return GL_QUERY_BUFFER;
		case buffer_target::shader_storage: return GL_SHADER_STORAGE_BUFFER;
		case buffer_target::texture: return GL_TEXTURE_BUFFER;
		case buffer_target::transform_feedback: return GL_TRANSFORM_FEEDBACK_BUFFER;
		case buffer_target::uniform: return GL_UNIFORM_BUFFER;
		default:
			throw logic_error{"bad cast, unknown buffer target"};
	}
}

GLenum opengl_cast(render_primitive p)
{
	switch (p)
	{
		case render_primitive::points: return GL_POINTS;
		case render_primitive::line_strip: return GL_LINE_STRIP;
		case render_primitive::line_loop: return GL_LINE_LOOP;
		case render_primitive::lines: return GL_LINES;
		case render_primitive::line_strip_adjacency: return GL_LINE_STRIP_ADJACENCY;
		case render_primitive::lines_adjacency: return GL_LINES_ADJACENCY;
		case render_primitive::triangle_strip: return GL_TRIANGLE_STRIP;
		case render_primitive::triangle_fan: return GL_TRIANGLE_FAN;
		case render_primitive::triangles: return GL_TRIANGLES;
		case render_primitive::triangle_strip_adjacency: return GL_TRIANGLE_STRIP_ADJACENCY;
		case render_primitive::triangles_adjacency: return GL_TRIANGLES_ADJACENCY;
		case render_primitive::patches: return GL_PATCHES;
		default:
			throw logic_error{"bad cast, unknown render primitive"};
	}
}

}  // gl
