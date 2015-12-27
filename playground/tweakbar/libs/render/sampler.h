#pragma once
#include <GL/glew.h>
#include "render/detail/texture_types.h"

namespace detail {

//! \saa #glTexParameter
template <typename Self>
class sampler_parameters
{
public:	
	virtual ~sampler_parameters() {}
	texture_wrap wrap_s() const {return _wraps;}
	texture_wrap wrap_t() const {return _wrapt;}
	texture_wrap wrap_r() const {return _wrapr;}
	texture_filter min() const {return _min;}
	texture_filter mag() const {return _mag;}
	unsigned int border_type() const {return _border_type;}
	GLint const * border_i() const {return _border.i;}
	GLfloat const * border_f() const {return _border.f;}
	GLint const * border_Ii() const {return _border.i;}
	GLuint const * border_Iui() const {return _border.ui;}
	GLfloat lod_min() const {return _lod_min;}
	GLfloat lod_max() const {return _lod_mag;}
	GLfloat lod_bias() const {return _lod_bias;}
	compare_function compare_func() const {return _compare_func;}
	GLfloat max_anisotropy_ext() const {return _max_anisotropy;}

	Self & wrap_s(texture_wrap w) {_wraps = w; return *_this;}
	Self & wrap_t(texture_wrap w) {_wrapt = w; return *_this;}
	Self & wrap_r(texture_wrap w) {_wrapr = w; return *_this;}
	Self & min(texture_filter f) {_min = f; return *_this;}
	Self & mag(texture_filter f) {_mag = f; return *_this;}
	Self & border_i(GLint r, GLint g, GLint b, GLint a = 0);
	Self & border_f(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 0.0f);
	Self & border_Ii(GLint r, GLint g, GLint b, GLint a = 0);
	Self & border_Iui(GLuint r, GLuint g, GLuint b, GLuint a = 0);
	Self & lod_min(GLfloat v) {_lod_min = v; return *_this;}
	Self & lod_max(GLfloat v) {_lod_mag = v; return *_this;}
	Self & lod_bias(GLfloat v) {_lod_bias = v; return *_this;}
	Self & compare_func(compare_function f) {_compare_func = f; return *_this;}
	Self & max_anisotropy_ext(GLfloat a) {_max_anisotropy = a; return *_this;}

protected:
	sampler_parameters(Self * p);

private:
	texture_wrap _wraps;
	texture_wrap _wrapt;
	texture_wrap _wrapr;
	texture_filter _min;
	texture_filter _mag;

	union {
		GLint i[4];
		GLuint ui[4];
		GLfloat f[4];
	} _border;

	unsigned int _border_type;
	GLfloat _lod_min;
	GLfloat _lod_mag;
	GLfloat _lod_bias;
	GLfloat _max_anisotropy;
	compare_function _compare_func;

	Self * _this;
};  // parameters

}  // detail

/*! Wrapper okolo funkcii #glGenSamplers, #glSamplerParameterf a #glDeleteSamplers.
\ingroup render */
class sampler
{
public:
	//! \saa #glTexParameter
	struct parameters : public detail::sampler_parameters<parameters>
	{
		parameters() : detail::sampler_parameters<parameters>(this)
		{}
	};

	sampler(parameters const & params);
	~sampler();
	GLuint id() const {return _id;}

private:
	GLuint _id;
	parameters _params;
};


namespace detail {

template <typename Self>
sampler_parameters<Self>::sampler_parameters(Self * p)
	: _this(p)
{
	_wraps = _wrapt = _wrapr = texture_wrap::CLAMP_TO_EDGE;
	_min = texture_filter::NEAREST;
	_mag = texture_filter::LINEAR;
	_border_type = 0;
	_lod_min = -1000.0f;
	_lod_mag = 1000.0f;
	_lod_bias = 0.0f;
	_max_anisotropy = 1.0f;
	_compare_func = compare_function::ALWAYS;
	_border.i[0] = 0;
	_border.i[1] = 0;
	_border.i[2] = 0;
	_border.i[3] = 0;
}

template <typename Self>
Self & sampler_parameters<Self>::border_i(GLint r, GLint g, GLint b, GLint a)
{
	_border.i[0] = r;
	_border.i[1] = g;
	_border.i[2] = b;
	_border.i[3] = a;
	return *_this;
}

template <typename Self>
Self & sampler_parameters<Self>::border_f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{	
	_border.f[0] = r;
	_border.f[1] = g;
	_border.f[2] = b;
	_border.f[3] = a;
	return *_this;
}

template <typename Self>
Self & sampler_parameters<Self>::border_Ii(GLint r, GLint g, GLint b, GLint a)
{
	_border.i[0] = r;
	_border.i[1] = g;
	_border.i[2] = b;
	_border.i[3] = a;
	return *_this;
}

template <typename Self>
Self & sampler_parameters<Self>::border_Iui(GLuint r, GLuint g, GLuint b, GLuint a)
{
	_border.ui[0] = r;
	_border.ui[1] = g;
	_border.ui[2] = b;
	_border.ui[3] = a;
	return *_this;
}

}  // detail
