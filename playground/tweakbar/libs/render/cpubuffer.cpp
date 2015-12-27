#include "cpubuffer.h"
#include <GL/glew.h>

void cpubuffer::bind(int target) const
{
	glBindBuffer(target, 0);
	assert(glGetError() == GL_NO_ERROR);
}

void * cpubuffer::data(int offset) const
{
	return (void *)((char *)_data + offset);
}
