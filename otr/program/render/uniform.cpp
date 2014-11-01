#include "render/uniform.hpp"
#include "render/program.hpp"
#include "render/texture.hpp"

void uniform_sampler::set_sampler(ptr<sampler> s)
{
	_sampler = s;
	if (_prog && _prog->current())
		set_value();
}

void uniform_sampler::set(ptr<texture> value)
{
	if (_prog)
	{
		if (_value)
			_value->remove_user(_prog->id());

		if (value)
			value->append_user(_prog->id());
	}

	_value = value;
	if (_prog && _prog->current())
		set_value();
}

void uniform_sampler::set_value(ptr<any_value> v)
{
	ptr<any_value_sampler> vs = std::dynamic_pointer_cast<any_value_sampler>(v);
	set(vs->get());
}

void uniform_sampler::set_value()
{
	if (_value && _location != -1 && program::CURRENT)
	{
		GLint new_unit = _value->bind_to_texture_unit(_sampler, program::CURRENT->_program_id);  // TODO: namiesto zoznamu programou tu davam iba jeden
		assert(new_unit >= 0);
		if (new_unit != _unit)
			glUniform1i(_location, new_unit);
		_unit = new_unit;
		assert(glGetError() == GL_NO_ERROR);
	}
	else
		_unit = -1;
}

template<>
void uniform1f::set_value()
{
	glUniform1f(_location, _value);
}

template<>
void uniform1d::set_value()
{
	glUniform1d(_location, _value);
}

template<>
void uniform1i::set_value()
{
	glUniform1i(_location, _value);
}

template<>
void uniform1ui::set_value()
{
	glUniform1ui(_location, _value);
}

template<>
void uniform1b::set_value()
{
	glUniform1ui(_location, _value ? 1 : 0);
}

template<>
void uniform2f::set_value()
{
	glUniform2f(_location, _value.x, _value.y);
}

template<>
void uniform2d::set_value()
{
	glUniform2d(_location, _value.x, _value.y);
}

template<>
void uniform2i::set_value()
{
	glUniform2i(_location, _value.x, _value.y);
}

template<>
void uniform2ui::set_value()
{
	glUniform2ui(_location, _value.x, _value.y);
}

template<>
void uniform2b::set_value()
{
	glUniform2ui(_location, _value.x ? 1 : 0, _value.y ? 1 : 0);
}

template<>
void uniform3f::set_value()
{
	glUniform3f(_location, _value.x, _value.y, _value.z);
}

template<>
void uniform3d::set_value()
{
	glUniform3d(_location, _value.x, _value.y, _value.z);
}

template<>
void uniform3i::set_value()
{
	glUniform3i(_location, _value.x, _value.y, _value.z);
}

template<>
void uniform3ui::set_value()
{
	glUniform3ui(_location, _value.x, _value.y, _value.z);
}

template<>
void uniform3b::set_value()
{
	glUniform3ui(_location, _value.x ? 1 : 0, _value.y ? 1 : 0, _value.z ? 1 : 0);
}

template<>
void uniform4f::set_value()
{
	glUniform4f(_location, _value.x, _value.y, _value.z, _value.w);
}

template<>
void uniform4d::set_value()
{
	glUniform4d(_location, _value.x, _value.y, _value.z, _value.w);
}

template<>
void uniform4i::set_value()
{
	glUniform4i(_location, _value.x, _value.y, _value.z, _value.w);
}

template<>
void uniform4ui::set_value()
{
	glUniform4ui(_location, _value.x, _value.y, _value.z, _value.w);
}

template<>
void uniform4b::set_value()
{
	glUniform4ui(_location, _value.x ? 1 : 0, _value.y ? 1 : 0, _value.z ? 1 : 0, _value.w ? 1 : 0);
}

template<>
void uniform_matrix2f::set_value()
{
	glUniformMatrix2fv(_location, 1, GL_FALSE, _value);
}

template<>
void uniform_matrix2d::set_value()
{
	glUniformMatrix2dv(_location, 1, GL_FALSE, _value);
}

template<>
void uniform_matrix<uniform_type::mat3f, GLfloat, 3, 3>::set_value()
{
	glUniformMatrix3fv(_location, 1, GL_FALSE, _value);
}

template<>
void uniform_matrix<uniform_type::mat3d, GLdouble, 3, 3>::set_value()
{
	glUniformMatrix3dv(_location, 1, GL_FALSE, _value);
}

template<>
void uniform_matrix<uniform_type::mat4f, GLfloat, 4, 4>::set_value()
{
	glUniformMatrix4fv(_location, 1, GL_FALSE, _value);
}

template<>
void uniform_matrix<uniform_type::mat4d, GLdouble, 4, 4>::set_value()
{
	glUniformMatrix4dv(_location, 1, GL_FALSE, _value);
}

template<>
void uniform_matrix2x3f::set_value()
{
	glUniformMatrix2x3fv(_location, 1, GL_FALSE, _value);
}

template<>
void uniform_matrix2x3d::set_value()
{
	glUniformMatrix2x3dv(_location, 1, GL_FALSE, _value);
}

template<>
void uniform_matrix2x4f::set_value()
{
	glUniformMatrix2x4fv(_location, 1, GL_FALSE, _value);
}

template<>
void uniform_matrix2x4d::set_value()
{
	glUniformMatrix2x4dv(_location, 1, GL_FALSE, _value);
}

template<>
void uniform_matrix3x2f::set_value()
{
	glUniformMatrix3x2fv(_location, 1, GL_FALSE, _value);
}

template<>
void uniform_matrix3x2d::set_value()
{
	glUniformMatrix3x2dv(_location, 1, GL_FALSE, _value);
}

template<>
void uniform_matrix3x4f::set_value()
{
	glUniformMatrix3x4fv(_location, 1, GL_FALSE, _value);
}

template<>
void uniform_matrix3x4d::set_value()
{
	glUniformMatrix3x4dv(_location, 1, GL_FALSE, _value);
}

template<>
void uniform_matrix4x2f::set_value()
{
	glUniformMatrix4x2fv(_location, 1, GL_FALSE, _value);
}

template<>
void uniform_matrix4x2d::set_value()
{
	glUniformMatrix4x2dv(_location, 1, GL_FALSE, _value);
}

template<>
void uniform_matrix4x3f::set_value()
{
	glUniformMatrix4x3fv(_location, 1, GL_FALSE, _value);
}

template<>
void uniform_matrix4x3d::set_value()
{
	glUniformMatrix4x3dv(_location, 1, GL_FALSE, _value);
}
