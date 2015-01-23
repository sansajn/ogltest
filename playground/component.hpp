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

class mesh_renderer : public game_component
{
public:
	mesh_renderer(ptr<mesh> m, ptr<material> t) : _mesh(m), _material(t) {}

	void render(game_shader & shader, renderer & rend) override;

private:
	ptr<mesh> _mesh;
	ptr<material> _material;
};
