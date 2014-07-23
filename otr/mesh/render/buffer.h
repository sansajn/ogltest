#pragma once

#include <GL/glew.h>

/*! Zjednodušený gpu-buffer (ork::GPUBuffer).
V podstate sa jedná o wrapper funkcií glGenBuffers a glBufferData.
\saa glGenBuffers, glBufferData, glBuffSubData */
class gpubuffer
{
public:
	gpubuffer();
	~gpubuffer();
	GLuint id() const {return _buffid;}
	int size() const {return _size;}
	void data(int size, void const * data, GLenum u = GL_STATIC_DRAW);
	void * data(int offset) const {return ((char *)NULL + offset);}
	void subdata(int offset, int size, void * data);
	bool reserve(int size, GLenum u = GL_STATIC_DRAW);
	void bind(GLenum target) const;
	void unbind(GLenum target) const;

private:
	int _size;
	GLuint _buffid;
};

typedef gpubuffer buffer;
