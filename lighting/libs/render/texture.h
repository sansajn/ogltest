#pragma once
#include <map>
#include <vector>
#include <GL/glew.h>
#include "core/ptr.hpp"
#include "render/sampler.h"
#include "render/detail/texture_types.h"

/*! Abstrakcia pre texturu.
\ingroup render */
class texture
{
public:
	virtual ~texture();
	GLuint id() const {return _id;}
	GLuint target() const {return _target;}
	texture_format format() const;
	texture_internal_format internal_format() const {return _internal_format;}

	// unit and program api
	void append_user(GLuint program_id) const;
	void remove_user(GLuint program_id) const;
	bool used_by(GLuint program_id) const;

	int bind_to_texture_unit() const;
	int bind_to_texture_unit(ptr<sampler> s, GLuint program_id) const;  //!< \param s can be nullptr
	void generate_mipmap() {}

	class parameters : public detail::sampler_parameters<parameters>
	{
	public:
		parameters();

		GLint level_min() const {return _lmin;}  //!< lowest mipmap level
		GLint level_max() const {return _lmax;}
		char const * swizzle() const {return _swizzle;}

		parameters & swizzle(char r, char g, char b, char a);
		parameters & level_min(GLint v) {_lmin = v; return *this;}
		parameters & level_max(GLint v) {_lmax = v; return *this;}

	private:		
		GLint _lmin;
		GLint _lmax;
		char _swizzle[4];
	};

protected:
	texture(GLuint texture_target);
	texture(GLuint target, texture_internal_format fi, parameters const & params);

private:
	GLuint _id;  //!< texture id
	GLuint _target;  //!< texture target
	texture_internal_format _internal_format;
	parameters _params;

	mutable std::vector<GLuint> _program_ids;
	mutable std::map<GLuint, int> _current_texture_units;  // sampler-id, unit

	friend class texture_unit;
	friend class texture_unit_manager;
};  // texture
