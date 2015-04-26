#include "mesh.hpp"
#include <memory>
#include <vector>
#include <cassert>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GL/glew.h>

using std::unique_ptr;

void copy_to_buffer(vertex const & v, float * & buf);

mesh::mesh() : _bufs{0,0}, _size(0)
{}

mesh::mesh(std::string const & fname) : _bufs{0,0}, _size(0)
{
	read(fname);
}

mesh::mesh(std::vector<vertex> const & verts, std::vector<unsigned> const & indices)
	: _bufs{0,0}, _size(0)
{
	create(verts, indices);
}

void mesh::create(std::vector<vertex> const & verts, std::vector<unsigned> const & indices)
{
	free();

	glGenBuffers(2, _bufs);

	// vbo
	unsigned vbufsize = verts.size() * (3+2+3+3);
	unique_ptr<float []> vbobuf(new float[vbufsize]);

	float * vbuf = vbobuf.get();
	for (auto v : verts)
		copy_to_buffer(v, vbuf);

	glBindBuffer(GL_ARRAY_BUFFER, _bufs[0]);
	glBufferData(GL_ARRAY_BUFFER, vbufsize*sizeof(float), vbobuf.get(), GL_STATIC_DRAW);

	vbobuf.reset();

	// ibo
	unique_ptr<unsigned []> ibobuf(new unsigned[indices.size()]);

	unsigned * ibuf = ibobuf.get();
	for (auto idx : indices)
		*ibuf++ = idx;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _bufs[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned), ibobuf.get(), GL_STATIC_DRAW);

	_size = indices.size();

	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

mesh::mesh(mesh && lhs)
{
	_bufs[0] = lhs._bufs[0];
	_bufs[1] = lhs._bufs[1];
	_size = lhs._size;
	lhs._bufs[0] = lhs._bufs[1] = 0;
}

mesh::~mesh()
{
	free();
}

void mesh::read(std::string const & fname)
{
	free();

	_bufs[0] = _bufs[1] = 0;

	Assimp::Importer importer;
	aiScene const * scene = importer.ReadFile(fname.c_str(),
		aiProcess_Triangulate|aiProcess_GenSmoothNormals|aiProcess_CalcTangentSpace|aiProcess_JoinIdenticalVertices);

	if (!scene)
		throw std::exception();  // TODO: specify exception (can't load mesh file)

	assert(scene->mNumMeshes > 0 && "model neobsahuje ziadnu mrizeku");

	aiMesh & mesh = *scene->mMeshes[0];

	assert(mesh.mNumVertices > 0 && "mriezka neobsahuje ziadne vrcholy");

	// vbo
	unsigned vbufsize = mesh.mNumVertices * (3+2+3+3);  // position, uv, normal, tangent
	std::unique_ptr<float[]> vbobuf(new float[vbufsize]);

	float * fbuf = vbobuf.get();
	for (int i = 0; i < mesh.mNumVertices; ++i)
	{
		aiVector3D & v = mesh.mVertices[i];
		*fbuf++ = v.x;
		*fbuf++ = v.y;
		*fbuf++ = v.z;

		if (mesh.mTextureCoords[0])
		{
			aiVector3D & uv = mesh.mTextureCoords[0][i];
			*fbuf++ = uv.x;
			*fbuf++ = uv.y;
		}
		else  // texture uv not available
		{
			*fbuf++ = 0.0f;
			*fbuf++ = 0.0f;
		}

		aiVector3D & n = mesh.mNormals[i];
		*fbuf++ = n.x;
		*fbuf++ = n.y;
		*fbuf++ = n.z;

		if (mesh.mTangents)
		{
			aiVector3D & t = mesh.mTangents[i];
			*fbuf++ = t.x;
			*fbuf++ = t.y;
			*fbuf++ = t.z;
		}
		else  // tangents not available
		{
			*fbuf++ = 0.0f;
			*fbuf++ = 0.0f;
			*fbuf++ = 0.0f;
		}
	}

	glGenBuffers(2, _bufs);

	glBindBuffer(GL_ARRAY_BUFFER, _bufs[0]);
	glBufferData(GL_ARRAY_BUFFER, vbufsize*sizeof(float), vbobuf.get(), GL_STATIC_DRAW);

	vbobuf.reset();  // release vbo buffer

	// ibo
	unsigned ibufsize = mesh.mNumFaces*3;
	std::unique_ptr<unsigned[]> ibobuf(new unsigned[ibufsize]);

	unsigned * ibuf = ibobuf.get();
	for (int i = 0; i < mesh.mNumFaces; ++i)
	{
		aiFace & f = mesh.mFaces[i];
		*ibuf++ = f.mIndices[0];
		*ibuf++ = f.mIndices[1];
		*ibuf++ = f.mIndices[2];
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _bufs[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ibufsize*sizeof(unsigned), ibobuf.get(), GL_STATIC_DRAW);

	_size = mesh.mNumFaces*3;

	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void mesh::draw() const
{
	glEnableVertexAttribArray(0);  // position
	glEnableVertexAttribArray(1);  // uv
	glEnableVertexAttribArray(2);  // normal
	glEnableVertexAttribArray(3);  // tangent

	glBindBuffer(GL_ARRAY_BUFFER, _bufs[0]);

	unsigned vertex_size = (3+2+3+3) * sizeof(GL_FLOAT);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_size, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertex_size, (GLvoid *)(3*sizeof(GL_FLOAT)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, vertex_size, (GLvoid *)((3+2)*sizeof(GL_FLOAT)));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, vertex_size, (GLvoid *)((3+2+3)*sizeof(GL_FLOAT)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _bufs[1]);
	glDrawElements(GL_TRIANGLES, _size, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void mesh::free()
{
	assert(((_bufs[0] && _bufs[1]) || (!_bufs[0] && !_bufs[1])) && "one buffer is not allocated");

	if (_bufs[0] && _bufs[1])
		glDeleteBuffers(2, _bufs);

	assert(glGetError() == GL_NO_ERROR && "opengl error");
}

void mesh::operator=(mesh && lhs)
{
	_bufs[0] = lhs._bufs[0];
	_bufs[1] = lhs._bufs[1];
	_size = lhs._size;
	lhs._bufs[0] = lhs._bufs[1] = 0;
}

void copy_to_buffer(vertex const & v, float * & buf)
{
	*buf++ = v.position.x;
	*buf++ = v.position.y;
	*buf++ = v.position.z;
	*buf++ = v.uv.s;
	*buf++ = v.uv.t;
	*buf++ = v.normal.x;
	*buf++ = v.normal.y;
	*buf++ = v.normal.z;
	*buf++ = v.tangent.x;
	*buf++ = v.tangent.y;
	*buf++ = v.tangent.z;
}

mesh make_quad_xy()
{
	return make_quad_xy(glm::vec2(-1,-1), 2.0f);
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

	return mesh(verts, indices);
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

	return mesh(verts, indices);
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

	return mesh(verts, indices);
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

	return mesh(verts, indices);
}
