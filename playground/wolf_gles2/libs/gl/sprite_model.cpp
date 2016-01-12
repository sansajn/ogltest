#include "sprite_model.hpp"
#include <memory>
#include <Magick++.h>
#include "gl/shapes.hpp"

using std::move;
using std::unique_ptr;
using std::string;
using gles2::mesh;
using gles2::shader::program;

// sprite helpers

static void copy32(void const * src, int srcw, int srch, int dstx, int dsty, int dstw, int dsth, void * dst)
{
	uint32_t const * s = (uint32_t const *)src;
	for (int r = 0; r < srch; ++r)
	{
		uint32_t * d = (uint32_t *)dst + ((r+dsty) * dstw + dstx);
		for (int c = 0; c < srcw; ++c)
			*(d++) = *(s++);
	}
}

sprite_model::sprite_model(std::string files[], int n, int width, int height)
{
	_first = _cur = 0;
	_last = 1;
	_sprite_count = n;
	_mode = repeat_mode::once;
	_mesh = gl::make_quad_xy<mesh>();

/*
	// postupne citaj subory a ich obsah kopiruj do jedneho buffra
	int size = n*width*height*4;  // RGBA
	unique_ptr<uint8_t []> data{new uint8_t[size]};
	memset(data.get(), 0, size);

	for (int i = 0; i < n; ++i)
	{
		Magick::Image im{files[i].c_str()};
		im.flip();
		Magick::Blob imblob;
		im.write(&imblob, "RGBA");
		int x = (width-im.columns())/2;
		uint8_t * dst = data.get() + width*height*i*4;
		copy32(imblob.data(), im.columns(), im.rows(), x, 0, width, height, dst);
	}

	_sprites = texture2d_array(width, height, n, sized_internal_format::rgba8, pixel_format::rgba, pixel_type::ub8, data.get(), texture::parameters{}.filter(texture_filter::nearest));
*/

	int size = width*height*4;  // RGBA
	unique_ptr<uint8_t []> data{new uint8_t[size]};

	for (int i = 0; i < n; ++i)
	{
		Magick::Image im{files[i].c_str()};
		im.flip();
		Magick::Blob imblob;
		im.write(&imblob, "RGBA");
		int x = (width-im.columns())/2;
		memset(data.get(), 0, size);
		copy32(imblob.data(), im.columns(), im.rows(), x, 0, width, height, data.get());
		_sprites.emplace_back(width, height, gles2::pixel_format::rgba, gles2::pixel_type::ub8, data.get(), gles2::texture::parameters{}.filter(gles2::texture_filter::nearest));
	}
}

void sprite_model::update(float dt)
{
	_t += dt;
	if (_t >= _period)
	{
		++_cur;
		if (_cur == _last)
			_cur = _mode == repeat_mode::once ? _last-1 : _first;
		_t = _t - _period;
		assert(_t >= 0 && "zaporny cas");
	}
}

void sprite_model::render(program & p)
{
	_sprites[_cur].bind(0);
	p.uniform_variable("s", 0);
	_mesh.render();
}

void sprite_model::animation_sequence(int first, int last, repeat_mode m)
{
	_first = first;
	_last = last;
	_cur = first;
	_mode = m;
	_t = 0;
	assert(last <= _sprite_count && "out of range");
}

void sprite_model::operator=(sprite_model && other)
{
	_first = other._first;
	_last = other._last;
	_cur = other._cur;
	_sprite_count = other._sprite_count;
	_mode = other._mode;
	_mesh = move(other._mesh);
	_sprites = move(other._sprites);
	_period = other._period;
	_t = other._t;
}


//! texturovany model, bez osvetlenia (gles2 nema pole textur)
char const * default_sprite_model_shader_source = R"(
	#ifdef _VERTEX_
	uniform mat4 local_to_screen;
	attribute vec3 position;
	attribute vec2 texcoord;
	varying vec2 uv;
	void main() {
		uv = texcoord;
		gl_Position = local_to_screen * vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	precision mediump float;
	uniform sampler2D s;
	varying vec2 uv;
	void main() {
		gl_FragColor = texture2D(s, uv);
	}
	#endif
)";
