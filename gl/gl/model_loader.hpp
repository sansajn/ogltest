#pragma once
#include <string>
#include "mesh.hpp"
#include "model.hpp"

// TODO: osamostatni funkcie citjuc cisto mriezky (bez zavislosti na texture)

namespace gl {

mesh mesh_from_file(std::string const & fname, unsigned mesh_idx = 0);
mesh mesh_from_memory(void const * buf, unsigned len, char const * file_format);


struct model_loader_parameters
{
	std::string file_format;   // .tga, .png, .jpg, ...
	std::string diffuse_texture_postfix;
	std::string normal_texture_postfix;
	std::string height_texture_postfix;
	std::string diffuse_uniform_name;
	std::string normal_uniform_name;
	std::string height_uniform_name;
	unsigned diffuse_texture_bind_unit;
	unsigned normal_texture_bind_unit;
	unsigned height_texture_bind_unit;

	model_loader_parameters()
		: file_format{".png"}
		, diffuse_texture_postfix{""}
		, normal_texture_postfix{"_local"}
		, height_texture_postfix{"_h"}
		, diffuse_uniform_name{"diff_tex"}
		, normal_uniform_name{"norm_tex"}
		, height_uniform_name{"height_tex"}
		, diffuse_texture_bind_unit{0}
		, normal_texture_bind_unit{1}
		, height_texture_bind_unit{2}
	{}
};

model model_from_file(char const * fname, model_loader_parameters const & params = model_loader_parameters{});

}  // gl
