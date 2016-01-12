#pragma once
#include <vector>
#include <string>
#include <glm/vec3.hpp>
#include <Magick++.h>
#include "gles2/mesh_gles2.hpp"
#include "gles2/texture_gles2.hpp"
#include "gles2/program_gles2.hpp"
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
	gles2::mesh _mesh;  // level mesh (floor, ceil, walls)
	gles2::texture2d _walls;
	gles2::shader::program _prog;  // program renderujuci steny, zem a strop levelu
	glm::vec3 _player_pos;
	std::vector<body_object> _phys_walls;
	body_object _phys_ground;
	std::vector<door_object *> _doors;
	gles2::shader::program _door_prog;
	gles2::mesh _door_mesh;
	std::vector<medkit_object *> _medkits;
	gles2::shader::program _medkit_prog;
	std::vector<enemy_object *> _enemies;
	gles2::shader::program _enemy_prog;

	// TODO: strop a zem budu od meshu oddelene
};
