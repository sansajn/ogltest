#pragma once
#include <map>
#include <memory>
#include "gl/resource.hpp"
#include "gl/program.hpp"

namespace shader {

extern char const * flat_shader_source;
extern char const * flat_shaded_shader_source;
extern char const * flat_phong_shader_source;
extern char const * textured_shader_source;
extern char const * textured_phong_shader_source;
extern char const * normal_geometry_shader_source;
extern char const * tangent_geometry_shader_source;

enum class default_program_id
{
	flat_shader_source,
	flat_shaded_shader_source,
	flat_phong_shader_source,
	textured_shader_source,
	textured_phong_shader_source,
	normal_geometry_shader_source,
	tangent_geometry_shader_source,
	count
};

class default_program_manager : public resource_manager
{
public:
	default_program_manager() {}
	std::shared_ptr<program> get(default_program_id pid);  //!< create from default shaders

private:
	std::map<default_program_id, std::shared_ptr<program>> _shaders;
};

default_program_manager & default_shaders();

}  // shader
