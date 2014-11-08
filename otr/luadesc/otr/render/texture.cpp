#include "render/texture.hpp"
#include <vector>
#include <cassert>
#include <algorithm>
#include <GL/glew.h>
#include "render/program.hpp"
#include "render/detail/texture_cast.hpp"
#include "render/sampler.hpp"

using std::vector;

/*! Abstrakcia texturovacej jednotky.
TODO: zbav sa pointrou */
class texture_unit
{
public:
	texture_unit(GLuint unit);
	void bind(sampler const * s, texture const * tex, unsigned int time);
	bool used() const {return _current_texture_binding != nullptr;}
	unsigned int last_binding_time() const {return _binding_time;}
	texture const * current_texture_binding() const {return _current_texture_binding;}
	sampler const * current_sampler_binding() const {return _current_sampler_binding;}

private:
	GLuint _unit;
	unsigned int _binding_time;
	sampler const * _current_sampler_binding;
	texture const * _current_texture_binding;
};

//! spravuje/priraduje texturovacie jednotky
class texture_unit_manager
{
public:
	texture_unit_manager();
	~texture_unit_manager();

	void bind(int unit_idx, sampler const * s, texture const * tex);
	void unbind(texture const * tex);
	void unbind(sampler const * s);
	void unbind_all();
	int find_free_texture_unit(GLuint program_id);
	int max_texture_units();

private:
	vector<texture_unit *> _units;
	unsigned int _time;
	static int _max_units;
};

int texture_unit_manager::_max_units = 0;

static texture_unit_manager * TEXTURE_UNIT_MANAGER = nullptr;

//! \saa #glTexParameter
static GLenum texture_swizzle(char c);

texture::texture(GLuint texture_target)
	: _target(texture_target)
{
	if (!TEXTURE_UNIT_MANAGER)
		TEXTURE_UNIT_MANAGER = new texture_unit_manager();
}

texture::texture(GLuint target, texture_internal_format fi, parameters const & params)
	: texture(target)
{
	_internal_format = fi;
	_params = params;

	glGenTextures(1, &_id);	
	assert(_id > 0 && "unable to generate a texture name");

	bind_to_texture_unit();

	if (_target == GL_TEXTURE_BUFFER)
		return;

	glTexParameteri(_target, GL_TEXTURE_WRAP_S, ogl_cast(_params.wrap_s()));
	glTexParameteri(_target, GL_TEXTURE_WRAP_T, ogl_cast(_params.wrap_t()));
	glTexParameteri(_target, GL_TEXTURE_WRAP_R, ogl_cast(_params.wrap_r()));
	glTexParameteri(_target, GL_TEXTURE_MIN_FILTER, ogl_cast(_params.min()));
	glTexParameteri(_target, GL_TEXTURE_MAG_FILTER, ogl_cast(_params.mag()));

	switch (_params.border_type())
	{
		case 0:  // i
			glTexParameteriv(_target, GL_TEXTURE_BORDER_COLOR, _params.border_i());
			break;
		case 1:  // f
			glTexParameterfv(_target, GL_TEXTURE_BORDER_COLOR, _params.border_f());
			break;
		case 2:  // Ii
			glTexParameterIiv(_target, GL_TEXTURE_BORDER_COLOR, _params.border_Ii());
			break;
		case 3:  // Iui
			glTexParameterIuiv(_target, GL_TEXTURE_BORDER_COLOR, _params.border_Iui());
			break;
		default:
			assert(false && "unknow border color type");
	}

	if (_target != GL_TEXTURE_RECTANGLE)
	{
		glTexParameterf(_target, GL_TEXTURE_MIN_LOD, _params.lod_min());
		glTexParameterf(_target, GL_TEXTURE_MAX_LOD, _params.lod_max());
	}

	glTexParameterf(_target, GL_TEXTURE_LOD_BIAS, params.lod_bias());
	if (params.compare_func() != compare_function::ALWAYS)
	{
		glTexParameteri(_target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(_target, GL_TEXTURE_COMPARE_FUNC, ogl_cast(_params.compare_func()));
	}

	glTexParameterf(_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, _params.max_anisotropy_ext());
	glTexParameteri(_target, GL_TEXTURE_SWIZZLE_R, texture_swizzle(_params.swizzle()[0]));
	glTexParameteri(_target, GL_TEXTURE_SWIZZLE_G, texture_swizzle(_params.swizzle()[1]));
	glTexParameteri(_target, GL_TEXTURE_SWIZZLE_B, texture_swizzle(_params.swizzle()[2]));
	glTexParameteri(_target, GL_TEXTURE_SWIZZLE_A, texture_swizzle(_params.swizzle()[3]));

	if (_target != GL_TEXTURE_RECTANGLE)
	{
		glTexParameteri(_target, GL_TEXTURE_BASE_LEVEL, _params.level_min());
		glTexParameteri(_target, GL_TEXTURE_MAX_LEVEL, _params.level_max());
	}

	assert(glGetError() == GL_NO_ERROR);
}

texture::~texture()
{
	TEXTURE_UNIT_MANAGER->unbind(this);
	glDeleteTextures(1, &_id);
	assert(glGetError() == GL_NO_ERROR);
}

void texture::append_user(GLuint program_id) const
{
	assert(find(_program_ids.begin(), _program_ids.end(), program_id) == _program_ids.end()
		&& "program-id already in the list");

	_program_ids.push_back(program_id);
}

void texture::remove_user(GLuint program_id) const
{
	auto it = std::find(_program_ids.begin(), _program_ids.end(), program_id);
	assert(it != _program_ids.end() && "program-id not in the list");
	_program_ids.erase(it);
}

int texture::bind_to_texture_unit(ptr<sampler> s, GLuint program_id) const
{
	GLuint sampler_id = s ? s->id() : 0;
	auto it = _current_texture_units.find(sampler_id);

	int unit;
	if (it != _current_texture_units.end())
		unit = it->second;
	else
		unit = TEXTURE_UNIT_MANAGER->find_free_texture_unit(program_id);
	
	TEXTURE_UNIT_MANAGER->bind(unit, s.get(), this);

	return unit;
}

int texture::bind_to_texture_unit() const
{
	if (_current_texture_units.empty())
	{
		int unit;
		if (shader::program::CURRENT)
			unit = TEXTURE_UNIT_MANAGER->find_free_texture_unit(shader::program::CURRENT->id());
		else
			unit = TEXTURE_UNIT_MANAGER->find_free_texture_unit(0);

		assert(unit != -1);
		TEXTURE_UNIT_MANAGER->bind(unit, nullptr, this);
		return unit;
	}
	else
	{
		int unit = _current_texture_units.begin()->second;
		glActiveTexture(GL_TEXTURE0 + unit);
		return unit;
	}
}

bool texture::used_by(GLuint program_id) const
{
	return std::find(_program_ids.begin(), _program_ids.end(), program_id) != _program_ids.end();
}


texture_unit_manager::texture_unit_manager()
	: _units(max_texture_units()), _time(0)
{
	for (int i = 0; i < _units.size(); ++i)
		_units[i] = new texture_unit(i);
}

texture_unit_manager::~texture_unit_manager()
{
	for (auto u : _units)
		delete u;
}

int texture_unit_manager::find_free_texture_unit(GLuint program_id)
{
	// najdi nepouzitu jednotku
	for (int i = 0; i < _max_units; ++i)
		if (!_units[i]->used())
			return i;

	/* Ak ziadna nepouzita jednotka nie je, potom pouzi najdlhšie nepoužitú
	jednotku takú, ktorá sa nepoužíva žiadnym s programou program_ids. */
	int best_unit = -1;
	unsigned oldest_binding_time = _time;

	for (int i = 0; i < _max_units; ++i)
	{
		texture const * t = _units[i]->current_texture_binding();
		if (!t->used_by(program_id))
		{
			unsigned binding_time = _units[i]->last_binding_time();
			if (binding_time < oldest_binding_time || best_unit == -1)
			{
				best_unit = i;
				oldest_binding_time = binding_time;
			}
		}
	}

	assert(best_unit != -1 && "no more texture units available");

	return best_unit;
}

void texture_unit_manager::bind(int unit_idx, sampler const * s, texture const * tex)
{
	_units[unit_idx]->bind(s, tex, _time++);
}

void texture_unit_manager::unbind(texture const * tex)
{
	for (int i = 0; i < _max_units; ++i)
		if (_units[i]->current_texture_binding() == tex)
			_units[i]->bind(nullptr, nullptr, _time++);
}

void texture_unit_manager::unbind(sampler const * s)
{
	for (int i = 0; i < _max_units; ++i)
		if (_units[i]->current_sampler_binding() == s)
			_units[i]->bind(nullptr, nullptr, _time++);
}

void texture_unit_manager::unbind_all()
{
	for (int i = 0; i < _max_units; ++i)
		_units[i]->bind(nullptr, nullptr, 0);
	_time = 0;
}

int texture_unit_manager::max_texture_units()
{
	if (_max_units == 0)
	{
		GLint max_combined_texture_image_units;
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_combined_texture_image_units);
		_max_units = max_combined_texture_image_units;
		assert(_max_units > 0 && "error: no texture units detected");
	}
	return _max_units;
}

texture_unit::texture_unit(GLuint unit)
	: _unit(unit), _binding_time(0), _current_sampler_binding(nullptr), _current_texture_binding(nullptr)
{}

void texture_unit::bind(sampler const * s, texture const * tex, unsigned int time)
{
	_binding_time = time;
	
	glActiveTexture(GL_TEXTURE0 + _unit);
		
	GLuint sampler_id = s ? s->id() : 0;
	GLuint current_sampler_id = 
		_current_sampler_binding ? _current_sampler_binding->id() : 0;

	if (s != _current_sampler_binding)
	{
		glBindSampler(_unit, sampler_id);
		_current_sampler_binding = s;
	}

	if (tex != _current_texture_binding)
	{
		if (_current_texture_binding)
		{
			auto it = _current_texture_binding->_current_texture_units.find(
				current_sampler_id);

			assert(it != _current_texture_binding->_current_texture_units.end()
				&& "aktualny sampler nie je v zozname texture-unit textury");

			_current_texture_binding->_current_texture_units.erase(it);
			if (!tex || _current_texture_binding->target() != tex->target())
				glBindTexture(_current_texture_binding->target(), 0);
		}

		if (tex)
		{
			tex->_current_texture_units.insert(std::make_pair(sampler_id, _unit));
			glBindTexture(tex->target(), tex->id());
		}

		_current_texture_binding = tex;
	} // if

	assert(glGetError() == GL_NO_ERROR);
}

texture::parameters::parameters()
	: detail::sampler_parameters<parameters>(this), _lmin(0), _lmax(1000)
{
	_swizzle[0] = 'r';
	_swizzle[1] = 'g';
	_swizzle[2] = 'b';
	_swizzle[3] = 'a';
}

texture::parameters & texture::parameters::swizzle(char r, char g, char b, char a)
{
	_swizzle[0] = r;
	_swizzle[1] = g;
	_swizzle[2] = b;
	_swizzle[3] = a;
	return *this;
}

GLenum texture_swizzle(char c)
{
	switch (c)
	{
		case 'r':
			return GL_RED;
		case 'g':
			return GL_GREEN;
		case 'b':
			return GL_BLUE;
		case 'a':
			return GL_ALPHA;
		case '0':
			return GL_ZERO;
		case '1':
			return GL_ONE;
		default:
			throw std::exception();  // TODO specify
	}
}
