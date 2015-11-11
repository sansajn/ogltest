// casto pouzivane objekty v scene
#pragma once
#include <glm/matrix.hpp>
#include "gl/program.hpp"
#include "gl/colors.hpp"
#include "gl/label.hpp"

class axis_object
{
public:
	axis_object();
	void render(glm::mat4 const & world_to_screen);

private:
	shader::program _prog;  // TODO: make shareable
};

class light_object
{
public:
	light_object(glm::vec3 const & color = rgb::yellow);
	void render(glm::mat4 const & local_to_screen);

private:
	glm::vec3 _color;
	shader::program _prog;  // TODO: make shareable
};


template <typename Window>
class fps_object
{
public:
	fps_object(Window const & wnd, float update_rate = 1.0f);
	void update(float dt);
	void render() {_lbl.render();}

private:
	Window const & _wnd;
	ui::label<Window> _lbl;
	float _t = 0;
	float _update_period;
};

std::string const default_font_path = "/usr/share/fonts/truetype/freefont/FreeMono.ttf";

template <typename Window>
fps_object<Window>::fps_object(Window const & wnd, float update_rate)
	: _wnd{wnd}, _lbl{0, 0, wnd}, _update_period{1.0f/update_rate}
{
	_lbl.font(default_font_path, 16);
}

template <typename Window>
void fps_object<Window>::update(float dt)
{
	_t += dt;
	if (_t > _update_period)
	{
		std::string s{"fps: "};
		s += std::to_string(_wnd.fps());
		_lbl.text(s);
		_t = 0;
	}
}
