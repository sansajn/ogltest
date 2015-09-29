#include "mesh.hpp"
#include <memory>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <cassert>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GL/glew.h>

using std::unique_ptr;
using std::vector;
using std::string;
using std::ostringstream;
using std::runtime_error;

static void extract_scene(aiScene const & scene, vector<mesh> & result);
static void extract_mesh(aiMesh const & mesh, vector<float> & vbuf, vector<unsigned> & ibuf);


mesh::mesh()
	: _nindices{0}
{
	_gpu_buffer_id[vbo_idx] = _gpu_buffer_id[ibo_idx] = 0;
}

mesh::mesh(std::vector<float> const & vbuf, std::vector<unsigned> const & ibuf)
	: _nindices{0}
{
	_gpu_buffer_id[vbo_idx] = _gpu_buffer_id[ibo_idx] = 0;

	glGenBuffers(2, _gpu_buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, _gpu_buffer_id[vbo_idx]);
	glBufferData(GL_ARRAY_BUFFER, vbuf.size()*sizeof(float), vbuf.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _gpu_buffer_id[ibo_idx]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ibuf.size()*sizeof(unsigned), ibuf.data(), GL_STATIC_DRAW);

	_nindices = ibuf.size();

	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

mesh::~mesh()
{
	assert(((_gpu_buffer_id[vbo_idx] && _gpu_buffer_id[ibo_idx]) || (!_gpu_buffer_id[vbo_idx] && !_gpu_buffer_id[ibo_idx])) && "one buffer is not allocated");

	if (_gpu_buffer_id[vbo_idx] && _gpu_buffer_id[ibo_idx])  // TODO: co s touto podmienkou ?
		glDeleteBuffers(2, _gpu_buffer_id);

	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void mesh::draw() const
{
	// priradenie attributov do shaderu

	// TODO: tu potrebujem vediet, kolko mam 'attributov' (4:position,uv,normal,tangent)
	glEnableVertexAttribArray(0);  // position
	glEnableVertexAttribArray(1);  // uv
	glEnableVertexAttribArray(2);  // normal
	glEnableVertexAttribArray(3);  // tangent

	glBindBuffer(GL_ARRAY_BUFFER, _gpu_buffer_id[vbo_idx]);

	// TODO: tu potrebujem vediet kolko prvkou ma vrchol (3+2+3+3) components_per_vertex
	unsigned vertex_size = (3+2+3+3) * sizeof(GL_FLOAT);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_size, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertex_size, (GLvoid *)(3*sizeof(GL_FLOAT)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, vertex_size, (GLvoid *)((3+2)*sizeof(GL_FLOAT)));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, vertex_size, (GLvoid *)((3+2+3)*sizeof(GL_FLOAT)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _gpu_buffer_id[ibo_idx]);
	glDrawElements(GL_TRIANGLES, _nindices, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

mesh::mesh(mesh && other)
{
	_gpu_buffer_id[vbo_idx] = other._gpu_buffer_id[vbo_idx];
	_gpu_buffer_id[ibo_idx] = other._gpu_buffer_id[ibo_idx];
	_nindices = other._nindices;
	other._gpu_buffer_id[vbo_idx] = other._gpu_buffer_id[ibo_idx] = 0;
}

void mesh::operator=(mesh && other)
{
	_gpu_buffer_id[vbo_idx] = other._gpu_buffer_id[vbo_idx];
	_gpu_buffer_id[ibo_idx] = other._gpu_buffer_id[ibo_idx];
	_nindices = other._nindices;
	other._gpu_buffer_id[vbo_idx] = other._gpu_buffer_id[ibo_idx] = 0;
}

mesh mesh_from_file(string const & fname, unsigned mesh_idx)
{
	Assimp::Importer importer;
	aiScene const * scene = importer.ReadFile(fname,
		aiProcess_Triangulate|aiProcess_GenSmoothNormals|aiProcess_CalcTangentSpace|aiProcess_JoinIdenticalVertices);

	if (!scene)
		throw runtime_error{string{"assimp: "} + string{importer.GetErrorString()}};

	assert(scene->mNumMeshes > mesh_idx && "mesh index out of range");

	vector<float> vbuf;
	vector<unsigned> ibuf;
	extract_mesh(*scene->mMeshes[mesh_idx], vbuf, ibuf);

	return mesh{vbuf, ibuf};
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

	vector<float> vbuf;
	vector<unsigned> ibuf;
	extract_mesh(*scene->mMeshes[0], vbuf, ibuf);

	return mesh{vbuf, ibuf};
}

model::model(string const & fname)
{
	Assimp::Importer importer;
	aiScene const * scene = importer.ReadFile(fname,
		aiProcess_Triangulate|aiProcess_GenSmoothNormals|aiProcess_CalcTangentSpace|aiProcess_JoinIdenticalVertices);

	if (!scene)
		throw runtime_error{string{"assimp: "} + string{importer.GetErrorString()}};

	extract_scene(*scene, _meshes);
}

model::model(model && other)
	: _meshes{std::move(other._meshes)}
{}

void model::draw() const
{
	for (mesh const & m : _meshes)
		m.draw();
}

void model::operator=(model && other)
{
	swap(_meshes, other._meshes);
}

void extract_scene(aiScene const & scene, vector<mesh> & result)
{
	vector<float> vbuf;
	vector<unsigned> ibuf;

	for (int i = 0; i < scene.mNumMeshes; ++i)
	{
		extract_mesh(*scene.mMeshes[i], vbuf, ibuf);
		result.emplace_back(vbuf, ibuf);
	}
}

void extract_mesh(aiMesh const & mesh, vector<float> & vbuf, vector<unsigned> & ibuf)
{
	// vertices
	unsigned vbuf_size = mesh.mNumVertices * (3+2+3+3);  // position, uv, normal, tangent
	vbuf.resize(vbuf_size);

	float * vptr = vbuf.data();
	for (int i = 0; i < mesh.mNumVertices; ++i)
	{
		aiVector3D & v = mesh.mVertices[i];
		*vptr++ = v.x;
		*vptr++ = v.y;
		*vptr++ = v.z;

		if (mesh.mTextureCoords[0])
		{
			aiVector3D & uv = mesh.mTextureCoords[0][i];
			*vptr++ = uv.x;
			*vptr++ = uv.y;
		}
		else  // texture uv not available
		{
			*vptr++ = 0.0f;
			*vptr++ = 0.0f;
		}

		aiVector3D & n = mesh.mNormals[i];
		*vptr++ = n.x;
		*vptr++ = n.y;
		*vptr++ = n.z;

		if (mesh.mTangents)
		{
			aiVector3D & t = mesh.mTangents[i];
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
	unsigned ibuf_size = mesh.mNumFaces*3;  // predpoklada triangulaciu mriezky
	ibuf.resize(ibuf_size);

	unsigned * iptr = ibuf.data();
	for (int n = 0; n < mesh.mNumFaces; ++n)
	{
		aiFace & f = mesh.mFaces[n];
		*iptr++ = f.mIndices[0];
		*iptr++ = f.mIndices[1];
		*iptr++ = f.mIndices[2];
	}
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

	return mesh{vbuf, indices};
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
