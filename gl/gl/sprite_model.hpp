#pragma once
#include <string>
#include <boost/noncopyable.hpp>
#include "gl/mesh.hpp"
#include "gl/texture.hpp"
#include "gl/program.hpp"

class sprite_model : private boost::noncopyable  //!< sprite aninovany model
{
public:
	enum class repeat_mode {once, loop};

	sprite_model(std::string files[], int n, int width, int height);
	void update(float dt);
	void render(shader::program & p);
	void animation_sequence(int first, int last, repeat_mode m = repeat_mode::once);
	void frame_rate(float fps) {_period = 1.0f/fps;}

private:
	int _first, _last, _cur;
	int _sprite_count;
	repeat_mode _mode;
	gl::mesh _mesh;
	texture2d_array _sprites;
	float _period = 1.0f;
	float _t = 0.0f;
};

extern std::string const default_sprite_model_shader_source;
