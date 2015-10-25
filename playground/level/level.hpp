#pragma once
#include <vector>
#include <string>
#include <glm/vec3.hpp>
#include <Magick++.h>
#include "mesh.hpp"
#include "texture.hpp"
#include "program.hpp"
#include "camera.hpp"
#include "physics/physics.hpp"

class bitmap
{
public:
	void load(std::string const & fname) {_im.read(fname);}
	unsigned at(unsigned x, unsigned y) const;  //!< RGBA
	unsigned width() const {return _im.columns();}
	unsigned height() const {return _im.rows();}

private:
	Magick::Image _im;  // TODO: pristup k pixelom je pomaly, pouzi predvareny (RGBA) blob
};

class level
{
public:
	level();
	void render(gl::camera & c);
	glm::vec3 const & player_position() const;
	void link_with_world(rigid_body_world & world);  // apply_physics, link_with_world, connect_physics, ...

private:
	void generate_level(bitmap const & data);  // vygeneruje model a fyziku levelu

	bitmap _data;  // level data as bitmap
	gl::mesh _mesh;  // level mesh (floor, ceil, walls)
	texture2d _walls;
	shader::program _prog;  // program renderujuci level
	glm::vec3 _player_pos;
	std::vector<physics_object> _phys_walls;
	physics_object _phys_ground;

	// TODO: strop a zem budu od meshu oddelene
};

