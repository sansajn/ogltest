#include "render/sampler_uniform.hpp"
#include <cassert>
#include <GL/glew.h>

//sampler_uniform & sampler_uniform::operator=(ptr<texture> t)
//{
//	if (_t)
//		_t->remove_user(_prog->id());

//	_t = t;
//	_t->append_user(_prog->id());

//	if (!_prog->used())
//		return *this;

//	int unit = _t->bind_to_texture_unit(_s, _prog->id());

//	glUniform1i(location(), unit);

//	assert(glGetError() == GL_NO_ERROR && "unable to assign uniform variable");
//	return *this;
//}
