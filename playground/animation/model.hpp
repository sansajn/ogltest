#pragma once
#include <map>
#include <vector>
#include <string>
#include <memory>
#include "texture.hpp"
#include "mesh.hpp"
#include "program.hpp"

using gl::mesh;

class texture_manager
{
public:
	// get()
	// find()
	std::shared_ptr<texture2d> find(std::string const & key);  // texture_name
	void root_path(std::string const & path) {_root_path = path;}
	bool exists(std::string const & texture_name) const;

private:
	std::string get_texture_filename(std::string const & key) const;

	std::string _root_path;
	std::map<std::string, std::shared_ptr<texture2d>> _textures;
};

class textured_model
{
public:
	enum texture_type
	{
		none_texture = 0,
		diffuse_texture = 1,
		normal_texture = 2,
		height_texture = 4,
		specular_texture = 8,
		glow_texture = 16
	};

	textured_model() {}
	textured_model(textured_model && other);
	void render(shader::program & prog);
	void append_mesh(std::shared_ptr<mesh> m, std::string const & texture_id, int texture_compound = diffuse_texture);
	void operator=(textured_model && other);

	textured_model(textured_model const &) = delete;
	void operator=(textured_model const &) = delete;

private:
	std::vector<std::string> _texture_ids;
	std::vector<int> _texture_compound;  // texture_type
	std::vector<std::shared_ptr<mesh>> _meshes;
};

textured_model textured_model_from_file(std::string const & fname);

extern texture_manager texman;  //  global texture manager
