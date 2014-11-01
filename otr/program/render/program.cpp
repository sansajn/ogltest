#include "render/program.hpp"
#include <cassert>
#include <exception>
#include <iostream>

program * program::CURRENT = nullptr;

program::program(ptr<module> m)
{
	std::vector<ptr<module>> mods;
	mods.push_back(m);
	init(mods);
}

void program::init(std::vector<ptr<module>> & modules)
{
	_modules = modules;
	_program_id = glCreateProgram();
	assert(_program_id > 0);

	for (auto m : _modules)
	{
		m->_users.insert(this);
		if (m->vertex_shader_id() != -1)
			glAttachShader(_program_id, m->vertex_shader_id());
		if (m->fragment_shader_id() != -1)
			glAttachShader(_program_id, m->fragment_shader_id());
	}

	glLinkProgram(_program_id);

	init_uniforms();
}

void program::init_uniforms()
{
	GLint linked;
	glGetProgramiv(_program_id, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE)
	{
		GLint loglen;
		glGetProgramiv(_program_id, GL_INFO_LOG_LENGTH, &loglen);
		char * log = new char[loglen];
		GLsizei len;
		glGetProgramInfoLog(_program_id, GLsizei(loglen), &len, log);
		std::cout << log << "\n";
		delete [] log;
		assert(false && "program not linked");
		glDeleteProgram(_program_id);
		_program_id = 0;
		throw std::exception();
	}

	// TODO: uniform blocks, subroutines and arrays not supported

	GLint max_name_len;
	glGetProgramiv(_program_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_name_len);
	char * buf = new char[max_name_len];

	GLint nuniforms = 0;
	glGetProgramiv(_program_id, GL_ACTIVE_UNIFORMS, &nuniforms);

	for (GLuint i = 0; i < GLuint(nuniforms); ++i)
	{
		GLsizei len;
		glGetActiveUniformName(_program_id, i, GLsizei(max_name_len), &len, buf);

		GLint size;
		glGetActiveUniformsiv(_program_id, 1, &i, GL_UNIFORM_SIZE, &size);
		assert(size == 1 && "uniform arrays not supported");
		
		GLint offset = glGetUniformLocation(_program_id, buf);

		std::string uname(buf);
		GLint uoffset = offset;

		GLint type;
		GLint matrix_stride;
		GLint row_major;
		glGetActiveUniformsiv(_program_id, 1, &i, GL_UNIFORM_TYPE, &type);
		glGetActiveUniformsiv(_program_id, 1, &i, GL_UNIFORM_MATRIX_STRIDE, &matrix_stride);
		glGetActiveUniformsiv(_program_id, 1, &i, GL_UNIFORM_IS_ROW_MAJOR, &row_major);

		ptr<uniform> u;

		switch(type)
		{
			case GL_FLOAT:
				u.reset(new uniform1f(this, uname, GLuint(uoffset)));
				break;
			case GL_FLOAT_VEC2:
				u.reset(new uniform2f(this, uname, GLuint(uoffset)));
				break;
			case GL_FLOAT_VEC3:
				u.reset(new uniform3f(this, uname, GLuint(uoffset)));
				break;
			case GL_FLOAT_VEC4:
				u.reset(new uniform4f(this, uname, GLuint(uoffset)));
				break;
			// TODO: support double
			case GL_INT:
				u.reset(new uniform1i(this, uname, GLuint(uoffset)));
				break;
			case GL_INT_VEC2:
				u.reset(new uniform2i(this, uname, GLuint(uoffset)));
				break;
			case GL_INT_VEC3:
				u.reset(new uniform3i(this, uname, GLuint(uoffset)));
				break;
			case GL_INT_VEC4:
				u.reset(new uniform4i(this, uname, GLuint(uoffset)));
				break;
			// TODO: support unsigned int
			// TODO: support bool
			case GL_FLOAT_MAT2:
				u.reset(new uniform_matrix2f(this, uname, GLuint(uoffset), GLuint(matrix_stride), int(row_major)));
				break;
			case GL_FLOAT_MAT3:
				u.reset(new uniform_matrix3f(this, uname, GLuint(uoffset), GLuint(matrix_stride), int(row_major)));
				break;
			case GL_FLOAT_MAT4:
				u.reset(new uniform_matrix4f(this, uname, GLuint(uoffset), GLuint(matrix_stride), int(row_major)));
				break;
			// TODO: support matrix 2x3, 2x4, 3x2, 3x4, 4x2, 4x3
			case GL_SAMPLER_1D:
			case GL_SAMPLER_1D_SHADOW:
				u.reset(new uniform_sampler(uniform_type::sampler1d, this, uname, GLuint(uoffset)));
				break;
			case GL_SAMPLER_2D:
			case GL_SAMPLER_2D_SHADOW:
				u.reset(new uniform_sampler(uniform_type::sampler2d, this, uname, GLuint(uoffset)));
				break;
			default:
				assert(false && "unsupported uniform type");
				break;
		}  // switch (type ...

		_uniforms.insert(std::make_pair(uname, u));
		if (std::dynamic_pointer_cast<uniform_sampler>(u))
			_uniform_samplers.push_back(std::dynamic_pointer_cast<uniform_sampler>(u));
	}   // for (nuniforms ...


	delete [] buf;

	for (ptr<module> m : _modules)  // initial uniform values
	{
		for (auto & kv : m->_initial_values)
		{
			std::string const & name = kv.first;
			ptr<any_value> value = kv.second;

			auto it = _uniforms.find(name);
			if (it != _uniforms.end())
			{
				ptr<uniform> u = it->second;
				ptr<any_value_sampler> vs = std::dynamic_pointer_cast<any_value_sampler>(value);
				ptr<uniform_sampler> us = std::dynamic_pointer_cast<uniform_sampler>(u);
				assert(u->name() == value->name());
				if (u->type() == value->type() || (us && vs))
					u->set_value(value);
			}
		}  // for (named_value ...
	}  // for (m ...

	assert(glGetError() == 0);
}

program::~program()
{
	if (CURRENT == this)
		CURRENT = nullptr;

	if (_program_id != 0)
	{
		update_texture_users(false);
		update_uniforms(nullptr);
	}

	for (auto m : _modules)
		m->_users.erase(this);

	if (_program_id > 0)
		glDeleteProgram(_program_id);
}

std::vector<ptr<uniform>> program::uniforms() const
{
	std::vector<ptr<uniform>> result;
	for (auto u : _uniforms)
		result.push_back(u.second);
	return result;
}

bool program::check_samplers()
{
	for (auto s : _uniform_samplers)
	{
		if (s->_location != -1 && s->get())
		{
			std::cout << "sampler not bound " << s->name() << "\n";
			return false;
		}
	}
	return true;
}

void program::set()
{
	if (CURRENT != this)
	{
		CURRENT = this;
		glUseProgram(_program_id);
		bind_textures();
		// TODO: pipeline support not implemente
	}
}

void program::bind_textures()
{
	for (auto s : _uniform_samplers)
		s->set_value();
	assert(glGetError() == GL_NO_ERROR);
}

void program::update_texture_users(bool add)
{
	for (auto s : _uniform_samplers)
	{
		ptr<texture> t = s->get();
		if (t)
		{
			if (add)
				t->append_user(s->_prog->id());
			else
			{
				t->remove_user(s->_prog->id());
				s->_unit = -1;
			}
		}
	}
}

void program::update_uniforms(program * owner)
{
	_uniform_samplers.clear();
	for (auto const & kv : _uniforms)
	{
		ptr<uniform> u = kv.second;
		ptr<uniform_sampler> us = std::dynamic_pointer_cast<uniform_sampler>(u);
		if (us)
			_uniform_samplers.push_back(us);
		u->_prog = owner;
	}
}

bool program::current() const
{
	return (CURRENT == this);
}
