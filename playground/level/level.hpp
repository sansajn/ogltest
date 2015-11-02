#pragma once
#include <vector>
#include <string>
#include <glm/vec3.hpp>
#include <Magick++.h>
#include "gl/mesh.hpp"
#include "gl/texture.hpp"
#include "gl/program.hpp"
#include "gl/camera.hpp"
#include "physics/physics.hpp"
#include "door.hpp"

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
	~level();
	void update(float dt);
	void render(gl::camera & c);
	glm::vec3 const & player_position() const;
	door * find_door();
	door * find_door(btTransform const & player);
	void link_with(rigid_body_world & world);

private:
	void generate_level(bitmap const & data);  // vygeneruje model a fyziku levelu

	bitmap _data;  // level data as bitmap
	gl::mesh _mesh;  // level mesh (floor, ceil, walls)
	texture2d _walls;
	shader::program _prog;  // program renderujuci level
	glm::vec3 _player_pos;
	std::vector<physics_object> _phys_walls;
	physics_object _phys_ground;
	std::vector<door *> _doors;
	shader::program _door_prog;
	gl::mesh _door_mesh;

	// TODO: strop a zem budu od meshu oddelene
};
