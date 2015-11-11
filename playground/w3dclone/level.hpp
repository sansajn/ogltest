#pragma once
#include <vector>
#include <string>
#include <glm/vec3.hpp>
#include <Magick++.h>
#include "gl/mesh.hpp"
#include "gl/texture.hpp"
#include "gl/program.hpp"
#include "gl/camera.hpp"
#include "medkit_world.hpp"

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

class level : private boost::noncopyable
{
public:
	level();
	~level();
	void update(float dt);
	void render(gl::camera & c);
	void link_with(medkit_world & world);
	glm::vec3 const & player_position() const;
	door_object * find_door(btTransform const & player, rigid_body_world & world);
	void remove_medkit(btCollisionObject * obj);

	// low level
	std::vector<enemy_object *> * enemies() {return &_enemies;}

private:
	void generate_level(bitmap const & data);  // vygeneruje model a fyziku levelu

	bitmap _data;  // level data as bitmap
	gl::mesh _mesh;  // level mesh (floor, ceil, walls)
	texture2d _walls;
	shader::program _prog;  // program renderujuci steny, zem a strop levelu
	glm::vec3 _player_pos;
	std::vector<body_object> _phys_walls;
	body_object _phys_ground;
	std::vector<door_object *> _doors;
	shader::program _door_prog;
	gl::mesh _door_mesh;
	std::vector<medkit_object *> _medkits;
	shader::program _medkit_prog;
	std::vector<enemy_object *> _enemies;
	shader::program _enemy_prog;

	// TODO: strop a zem budu od meshu oddelene
};
