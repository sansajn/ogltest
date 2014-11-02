#include "sampler.hpp"
#include <cassert>
#include <GL/glew.h>
#include "render/detail/texture_cast.hpp"

sampler::sampler(parameters const & params)
	: _params(params)
{
	glGenSamplers(1, &_id);
	assert(_id > 0 && "can't generate sampler name");
	
	glSamplerParameteri(_id, GL_TEXTURE_WRAP_S, ogl_cast(_params.wrap_s()));
	glSamplerParameteri(_id, GL_TEXTURE_WRAP_T, ogl_cast(_params.wrap_s()));
	glSamplerParameteri(_id, GL_TEXTURE_WRAP_R, ogl_cast(_params.wrap_r()));
	glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, ogl_cast(_params.min()));
	glSamplerParameteri(_id, GL_TEXTURE_MAG_FILTER, ogl_cast(_params.mag()));

	switch (_params.border_type())
	{
		case 0:  // i
			glSamplerParameteriv(_id, GL_TEXTURE_BORDER_COLOR, _params.border_i());
			break;
		case 1:  // f
			glSamplerParameterfv(_id, GL_TEXTURE_BORDER_COLOR, _params.border_f());
			break;
		case 2:  // Ii
			glSamplerParameterIiv(_id, GL_TEXTURE_BORDER_COLOR, _params.border_Ii());
			break;
		case 3:  // Iui
			glSamplerParameterIuiv(_id, GL_TEXTURE_BORDER_COLOR, _params.border_Iui());
			break;
		default:
			assert(false && "unknow border color type");
	}

	glSamplerParameterf(_id, GL_TEXTURE_MIN_LOD, _params.lod_min());
	glSamplerParameterf(_id, GL_TEXTURE_MAX_LOD, _params.lod_max());
	glSamplerParameterf(_id, GL_TEXTURE_LOD_BIAS, _params.lod_bias());

	if (_params.compare_func() != compare_function::ALWAYS)
	{
		glSamplerParameteri(_id, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glSamplerParameteri(_id, GL_TEXTURE_COMPARE_FUNC, ogl_cast(_params.compare_func()));
	}

	glSamplerParameterf(_id, GL_TEXTURE_MAX_ANISOTROPY_EXT, _params.max_anisotropy_ext());
}

sampler::~sampler()
{
	glDeleteSamplers(1, &_id);
}
