#pragma once
#include <vector>
#include <string>
#include <boost/noncopyable.hpp>
#include "gles2/mesh_gles2.hpp"
#include "gles2/texture_gles2.hpp"
#include "gles2/program_gles2.hpp"

//! sprite aninovany model
class sprite_model : private boost::noncopyable
{
public:
	enum class repeat_mode {once, loop};

	sprite_model() {}
	sprite_model(std::string files[], int n, int width, int height);
	void update(float dt);
	void render(gles2::shader::program & p);
	void animation_sequence(int first, int last, repeat_mode m = repeat_mode::once);
	void frame_rate(float fps) {_period = 1.0f/fps;}
	unsigned sprite_count() const {return _sprite_count;}
	float period() const {return _period;}

	void operator=(sprite_model && other);

private:
	int _first, _last, _cur;
	unsigned _sprite_count;
	repeat_mode _mode;
	gles2::mesh _mesh;
	std::vector<gles2::texture2d> _sprites;
	float _period = 1.0f;
	float _t = 0.0f;
};

extern char const * default_sprite_model_shader_source;
