#pragma once

#include <cstdint>

namespace gl {

class texture
{
public:
	// ! daj exception, ak nevies loadnut
	texture();
	texture(char const * filename);
	texture(int w, int h, int ch);
	texture(texture && rhs);
	~texture();

	bool load(char const * filename);
	bool load_heightmap(char const * filename);  // toto stransformovat do tvaru load()
	bool loaded() const {return _data;}
	int width() const {return _w;}
	int height() const {return _h;}
	int channels() const {return _ch;}
	uint8_t * data() const {return _data;}
	bool save(char const * filename);

	texture(texture const &) = delete;

private:	
	void free();

	int _w, _h, _ch;
	uint8_t * _data;
};

// toto ignoruje sposob akym k datam pristupujem (iterativne), ak by sa jednalo o nahodny pristup, potom je pristup optimalny
class tile_get
{
public:
	tile_get(gl::texture & t) : _t(t) {}
	uint8_t * operator()(int x, int y);

private:
	gl::texture & _t;
};

class scale_get  // nearest
{
public:
	scale_get(int w, int h, gl::texture & t) : _w(w), _h(h), _t(t)	{}
	uint8_t * operator()(int x, int y);

private:
	int _w, _h;
	gl::texture & _t;
};

}  // gl
