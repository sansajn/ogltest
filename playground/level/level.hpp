#pragma once
#include <string>
#include <glm/vec3.hpp>
#include <Magick++.h>
#include "mesh.hpp"
#include "texture.hpp"
#include "program.hpp"
#include "camera.hpp"

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

private:
	gl::mesh generate_level(bitmap const & data);

	bitmap _data;  // level data as bitmap
	gl::mesh _mesh;
	texture2d _walls;
	shader::program _prog;  // program renderujuci level
	glm::vec3 _player_pos;

	// TODO: strop a zem budu os meshu oddelene
};

