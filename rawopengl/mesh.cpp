#include "mesh.hpp"
#include <memory>
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
	open(fname);
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

void mesh::open(std::string const & fname)
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

		aiVector3D & uv = mesh.mTextureCoords[0][i];
		*fbuf++ = uv.x;
		*fbuf++ = uv.y;

		aiVector3D & n = mesh.mNormals[i];
		*fbuf++ = n.x;
		*fbuf++ = n.y;
		*fbuf++ = n.z;

		aiVector3D & t = mesh.mTangents[i];
		*fbuf++ = t.x;
		*fbuf++ = t.y;
		*fbuf++ = t.z;
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
}

void mesh::free()
{
	assert(((_bufs[0] && _bufs[1]) || (!_bufs[0] && !_bufs[1])) && "one buffer is not allocated");

	if (_bufs[0] && _bufs[1])
		glDeleteBuffers(2, _bufs);
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
