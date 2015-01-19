#pragma once
#include <map>
#include <string>
#include "ptr.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include "engine.hpp"

class camera : public game_component
{
public:
	camera(glm::mat4 const & proj) : _P(proj) {}
	camera(float fov, float aspect, float near, float far);

	glm::mat4 projection() const {return _P;}
	glm::mat4 view_projection() const;

	glm::vec3 right() const;
	glm::vec3 up() const;
	glm::vec3 forward() const;

private:
	glm::mat4 _P;
};

class material
{
public:
	material() {}
	material(ptr<texture> diffuse, float specular_intensity = 1.0f, float specular_power = 8.0f);

	float get_float(std::string const & name);
	glm::vec3 get_vector(std::string const & name);
	ptr<texture> get_texture(std::string const & name);

	void assoc_float(std::string const & name, float v) {_floats[name] = v;}
	void assoc_vector(std::string const & name, glm::vec3 const & v) {_vectors[name] = v;}
	void assoc_texture(std::string const & name, ptr<texture> t) {_textures[name] = t;}

private:
	std::map<std::string, ptr<texture>> _textures;
	std::map<std::string, float> _floats;
	std::map<std::string, glm::vec3> _vectors;
};

class mesh_renderer : public game_component
{
public:
	mesh_renderer(ptr<mesh> m, ptr<material> t) : _mesh(m), _material(t) {}

	void render(shader::program & prog, renderer & rend) override;

private:
	ptr<mesh> _mesh;
	ptr<material> _material;
};
