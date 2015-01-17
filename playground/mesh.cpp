#include "mesh.hpp"
#include <cassert>
#include <boost/filesystem/path.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GL/glew.h>
#include "utils.hpp"

namespace fs = boost::filesystem;

memory_stream & operator<<(memory_stream & s, aiVector3D const & v)
{
	s << v.x << v.y << v.z;
	return s;
}

mesh::mesh(std::string const & fname)
{
	_bufs[0] = _bufs[1] = 0;

	fs::path fpath("res/models");
	fpath /= fname;

	Assimp::Importer importer;
	aiScene const * scene = importer.ReadFile(fpath.c_str(),
															aiProcess_Triangulate|
															aiProcess_GenSmoothNormals|
															aiProcess_CalcTangentSpace|
															aiProcess_FlipUVs|
															aiProcess_JoinIdenticalVertices);
	if (!scene)
		throw std::exception();  // TODO: specify exception (can't load mesh file)

	assert(scene->mNumMeshes > 0 && "model neobsahuje ziadnu mrizeku");

	aiMesh & mesh = *scene->mMeshes[0];

	assert(mesh.mNumVertices > 0 && "mriezka neobsahuje ziadne vrcholy");

	// vbo
	unsigned vbufsize = mesh.mNumVertices * (3+2+3+3) * sizeof(float);  // position, uv, normal, tangent
	memory_stream vbuf(vbufsize);

	for (int i = 0; i < mesh.mNumVertices; ++i)
	{
		aiVector3D & uv = mesh.mTextureCoords[0][i];
		vbuf << mesh.mVertices[i];
		vbuf << uv.x << uv.y;
		vbuf << mesh.mNormals[i];
		vbuf << mesh.mTangents[i];
	}

	glGenBuffers(2, _bufs);

	glBindBuffer(GL_ARRAY_BUFFER, _bufs[0]);
	glBufferData(GL_ARRAY_BUFFER, vbufsize, vbuf.buf(), GL_STATIC_DRAW);

	vbuf.close();

	// ibo
	unsigned ibufsize = mesh.mNumFaces*3*sizeof(float);
	memory_stream ibuf(ibufsize);

	for (int i = 0; i < mesh.mNumFaces; ++i)
	{
		aiFace & f = mesh.mFaces[i];
		ibuf << f.mIndices[0] << f.mIndices[1] << f.mIndices[2];
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _bufs[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ibufsize, ibuf.buf(), GL_STATIC_DRAW);

	ibuf.close();

	_size = mesh.mNumFaces*3;
}

mesh::~mesh()
{
	glDeleteBuffers(2, _bufs);
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
