#pragma once
#include <string>
#include <tuple>
#include <utility>
#include <stdexcept>
#include <glm/vec2.hpp>
#include <GL/glew.h>

namespace ui {

struct window_error : public std::runtime_error
{
	window_error(std::string const & s) : std::runtime_error(s) {}
};

//! \sa basic_window
template <typename Impl>
class window : public Impl
{
public:
	using parameters = typename Impl::parameters;  // basic_window::parameters

	window(parameters const & p = parameters());
	virtual ~window() {}

	void reshape(int w, int h) override;

	unsigned width() const {return _w;}
	unsigned height() const {return _h;}
	float aspect_ratio() const {return float(_w)/float(_h);}
	glm::ivec2 center() const {return glm::ivec2(_w/2, _h/2);}

	void bind_as_render_target();

private:
	unsigned _w, _h;  //!< window geometry
};  // window

class event_handler
{
public:
	enum class button  //!< mouse buttons
	{
		left,
		middle,
		right,
		wheel_up,
		wheel_down,
		number_of_buttons
	};

	enum class state  //!< button states
	{
		down,
		up
	};

	enum class wheel
	{
		up,
		down
	};

	enum modifier  //! \note can be combination of modifiers
	{
		none = 0,
		shift = 1,
		ctrl = 2,
		alt = 4
	};

	enum class key  //!< special keys
	{
		caps_lock,
		f1,
		f2,
		f3,
		f4,
		f5,
		f6,
		f7,
		f8,
		f9,
		f10,
		f11,
		f12,
		print_screen,
		scroll_lock,
		pause,
		insert,
		home,
		page_up,
		end,
		page_down,
		right,
		left,
		down,
		up,
		unknown
	};

	virtual void display() {}
	virtual void reshape(int w, int h) {}
	virtual void idle() {}
	virtual void close() {}
	virtual void mouse_click(button b, state s, modifier m, int x, int y) {}
	virtual void mouse_motion(int x, int y) {}
	virtual void mouse_passive_motion(int x, int y) {}
	virtual void mouse_wheel(wheel w, modifier m, int x, int y) {}
	virtual void key_typed(unsigned char c, modifier m, int x, int y) {}
	virtual void key_released(unsigned char c, modifier m, int x, int y) {}
	virtual void special_key(key k, modifier m, int x, int y) {}
	virtual void special_key_released(key k, modifier m, int x, int y) {}
};  // event_handler

class basic_window : public event_handler
{
public:
	virtual ~basic_window() {}
	virtual void start() = 0;

	class parameters
	{
	public:
		using self = parameters;

		parameters();

		unsigned width() const {return _w;}
		unsigned height() const {return _h;}
		std::string const & name() const {return _name;}
		bool debug() const {return _debug;}
		std::pair<int, int> version() const {return _version;}

		self & size(unsigned w, unsigned h) {_w = w; _h = h; return *this;}
		self & name(std::string const & s) {_name = s; return *this;}
		self & debug(bool d) {_debug = d; return *this;}
		self & version(int major, int minor) {_version = std::make_pair(major, minor); return *this;}

	private:
		unsigned _w, _h;
		std::string _name;
		bool _debug;
		std::pair<int, int> _version;  // (major, minor)
	};
};  // basic_window


namespace detail {

class basic_glut_impl : public basic_window
{
public:
	basic_glut_impl(parameters const & p);
	~basic_glut_impl();

	void start() override;
	void display() override;

private:
	int _wid;  //!< window id
};

}  // detail

class glut_event_impl : public detail::basic_glut_impl  //!< glut window event mode implementation
{
public:
	using base = detail::basic_glut_impl;

	glut_event_impl(parameters const & p);

	void force_redisplay() const;
	void enable_idle() const;  //!< enables idle function to be called by glut engine
	void disable_idle() const;
};

class glut_pool_impl : public detail::basic_glut_impl  //!< glut pool mode window implementation (designed for games)
{
public:
	using base = detail::basic_glut_impl;

	glut_pool_impl(parameters const & p);
	~glut_pool_impl() = default;

	void start() override;
	virtual void input(float dt) {}
	virtual void update(float dt);
	void close() override;

	float fps() const {return std::get<0>(_fps);}
	std::tuple<float, float, float> const & fps_stats() const {return _fps;}  //!< \return returns (current, min, max) fps triplet

	class user_input  // keyboard and mouse input
	{
	public:
		user_input();

		void update();

		bool key(unsigned char c) const;
		bool key_up(unsigned char c) const;
		bool one_of_key(char const * s) const;
		bool one_of_key_up(char const * s) const;
		bool mouse(button b) const;
		bool mouse_up(button b) const;
		bool wheel_up(wheel w) const;

		glm::ivec2 const & mouse_position() const {return _mouse_pos;}

	private:
		void keyb_init();
		void mouse_init();
		void keyb_update();
		void mouse_update();

		static unsigned const NUM_KEYS = 256;
		bool _keys[NUM_KEYS];
		bool _keys_up[NUM_KEYS];
		glm::ivec2 _mouse_pos;
		bool _mouse_buttons[int(button::number_of_buttons)];
		bool _mouse_buttons_up[int(button::number_of_buttons)];

		friend class glut_pool_impl;
	};  // input

	user_input in;  //!< keyboard and mouse input

private:
	void mouse_motion(int x, int y) override;
	void mouse_passive_motion(int x, int y) override;
	void mouse_click(button b, state s, modifier m, int x, int y) override;
	void mouse_wheel(wheel w, modifier m, int x, int y) override;
	void key_typed(unsigned char c, modifier m, int x, int y) override;
	void key_released(unsigned char c, modifier m, int x, int y) override;

	std::tuple<float, float, float> _fps;  // (current, min, max)
	bool _closed = false;
};  // glut_pool_impl

namespace detail {

void glew_init();

}  // detail

template <typename Impl>
window<Impl>::window(parameters const & p) : Impl(p)
{
	_w = p.width();
	_h = p.height();
	detail::glew_init();
}

template <typename Impl>
void window<Impl>::reshape(int w, int h)
{
	Impl::reshape(w, h);
	_w = w;
	_h = h;
	glViewport(0, 0, w, h);
}

template <typename Impl>
void window<Impl>::bind_as_render_target()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glViewport(0, 0, _w, _h);
}

using glut_event_window = window<glut_event_impl>;
using glut_pool_window = window<glut_pool_impl>;
using glut_window = glut_pool_window;

}  // ui
