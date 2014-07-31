#pragma once
#include <GL/glew.h>
#include "render/buffer.h"
#include "render/detail/buffer_types.h"

/*! Zjednodušený gpu-buffer.
V podstate sa jedná o wrapper funkcií glGenBuffers a glBufferData.
\saa glGenBuffers, glBufferData, glBuffSubData
\ingroup render */
class gpubuffer : public buffer
{
public:
	gpubuffer();
	~gpubuffer();
	GLuint id() const {return _buffid;}
	int size() const {return _size;}
	void data(int size, void const * data, buffer_usage u = buffer_usage::STATIC_DRAW);
	void subdata(int offset, int size, void * data);	
	bool reserve(int size, buffer_usage u = buffer_usage::STATIC_DRAW);  //!< rezervuje potrbnu pamet (k pouzitiu zo #subdata)
	void bind(int target) const override;
	void unbind(int target) const override;
	void * data(int offset = 0) const override {return ((char *)NULL + offset);}

private:
	int _size;
	GLuint _buffid;
};
