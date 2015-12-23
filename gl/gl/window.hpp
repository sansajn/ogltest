#pragma once
#include <chrono>
#include <string>
#include <tuple>
#include <cassert>
#include <glm/vec2.hpp>
#include <GL/glew.h>

namespace ui {

template <template<class> class B, typename L>  // B : behaviour, L : layer
class window : public B<L>
{
public:
	using parameters = typename L::parameters;

	window(parameters const & params = parameters{});
	unsigned width() const {return _w;}
	unsigned height() const {return _h;}
	float aspect_ratio() const {return (float)_w/(float)_h;}
	glm::ivec2 center() const;
	void bind_as_render_target();

private:
	unsigned _w, _h;
};


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

	enum modifier  //!< key modifiers \note can be combination of modifiers
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


//! abstrakcia okennej vrstvy \sa glut_layer
class window_layer : public event_handler
{
public:
	class parameters
	{
	public:
		using self = parameters;

		parameters();

		self & geometry(unsigned w, unsigned h);
		self & name(std::string const & s);
		self & debug(bool d);
		self & version(int major, int minor);

		unsigned width() const {return _w;}
		unsigned height() const {return _h;}
		std::string const & name() const {return _name;}
		bool debug() const {return _debug;}
		std::pair<int, int> version() const {return _version;}

	private:
		unsigned _w, _h;
		std::string _name;
		bool _debug;
		std::pair<int, int> _version;  // (major, minor)
	};

	virtual ~window_layer() {}
	virtual void install_display_handler() {}
	virtual void main_loop() {}
	virtual void main_loop_event() {}
	virtual void swap_buffers() {}
	virtual int modifiers() {assert(0 && "unimplemented method"); return 0;}
};


template <typename L>  // L : window-layer implementation \sa glut_layer
class event_behaviour : public L
{
public:
	using parameters = typename L::parameters;

	event_behaviour(parameters const & params);
	void start() {L::main_loop();}
	void idle() override {/* TODO: uspi vlakno na 1/60s */}
};


template <typename L>
class pool_behaviour : public L
{
public:
	using parameters = typename L::parameters;
	using user_input = typename L::user_input;

	pool_behaviour(parameters const & params);
	void start();
	virtual void update(float dt);
	virtual void input(float dt) {}
	void close() override;
	float fps() const;
	std::tuple<float, float, float> const & fps_stats() const;

	user_input in;

private:
	void mouse_motion(int x, int y) override;
	void mouse_passive_motion(int x, int y) override;
	void mouse_click(event_handler::button b, event_handler::state s, event_handler::modifier m, int x, int y) override;
	void mouse_wheel(event_handler::wheel w, event_handler::modifier m, int x, int y) override;
	void key_typed(unsigned char c, event_handler::modifier m, int x, int y) override;
	void key_released(unsigned char c, event_handler::modifier m, int x, int y) override;

	std::tuple<float, float, float> _fps;  // (current, min, max)
	bool _closed = false;
};


template <template<class> class B, typename L>
window<B, L>::window(parameters const & params)
	: B<L>{params}
{
	_w = params.width();
	_h = params.height();
}

template <template<class> class B, typename L>
glm::ivec2 window<B, L>::center() const
{
	return glm::ivec2{_w/2, _h/2};
}

template <template<class> class B, typename L>
void window<B, L>::bind_as_render_target()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glViewport(0, 0, _w, _h);
}


template <typename L>
event_behaviour<L>::event_behaviour(parameters const & params)
	: L{params}
{
	L::install_display_handler();
}


template <typename L>
pool_behaviour<L>::pool_behaviour(parameters const & params)
	: L{params}, _fps{std::make_tuple(0.0f, 1e6f, 0.0f)}
{}

template <typename L>
void pool_behaviour<L>::start()
{
	using hclock = std::chrono::high_resolution_clock;
	hclock::time_point t_prev = hclock::now();

	while (true)
	{
		hclock::time_point now = hclock::now();
		hclock::duration d = now - t_prev;
		t_prev = now;
		float dt = std::chrono::duration_cast<std::chrono::milliseconds>(d).count() / 1000.0f;

		L::main_loop_event();
		if (_closed)
			break;

		input(dt);
		update(dt);
		this->display();

		in.update();
	}
}

template <typename L>
void pool_behaviour<L>::update(float dt)
{
	// fps statistics
	static float time_count = 0.0f;
	static unsigned frame_count = 0;

	time_count += dt;
	frame_count += 1;

	if (time_count > 1.0f)
	{
		float curr = frame_count/time_count;
		_fps = std::make_tuple(curr, std::min(std::get<1>(_fps), curr), std::max(std::get<2>(_fps), curr));
		frame_count = 0;
		time_count -= 1.0f;
	}
}

template <typename L>
void pool_behaviour<L>::close()
{
	_closed = true;
}

template <typename L>
float pool_behaviour<L>::fps() const
{
	return std::get<0>(_fps);
}

template <typename L>
std::tuple<float, float, float> const & pool_behaviour<L>::fps_stats() const
{
	return _fps;
}

template <typename L>
void pool_behaviour<L>::mouse_motion(int x, int y)
{
	in._mouse_pos = glm::ivec2{x,y};
}

template <typename L>
void pool_behaviour<L>::mouse_passive_motion(int x, int y)
{
	in._mouse_pos = glm::ivec2{x,y};
}

template <typename L>
void pool_behaviour<L>::mouse_click(event_handler::button b, event_handler::state s, event_handler::modifier m, int x, int y)
{
	if (s == event_handler::state::down)
		in._mouse_buttons[(int)b] = true;
	else
	{
		in._mouse_buttons[(int)b] = false;
		in._mouse_buttons_up[(int)b] = true;
	}
	in._mouse_pos = glm::ivec2{x,y};
}

template <typename L>
void pool_behaviour<L>::mouse_wheel(event_handler::wheel w, event_handler::modifier m, int x, int y)
{
	using eh = event_handler;

	if (w == eh::wheel::up)
		in._mouse_buttons_up[(int)eh::button::wheel_up] = true;

	if (w == eh::wheel::down)
		in._mouse_buttons_up[(int)eh::button::wheel_down] = true;
}

template <typename L>
void pool_behaviour<L>::key_typed(unsigned char c, event_handler::modifier m, int x, int y)
{
	in._keys[c] = true;
}

template <typename L>
void pool_behaviour<L>::key_released(unsigned char c, event_handler::modifier m, int x, int y)
{
	in._keys[c] = false;
	in._keys_up[c] = true;
}

}  // experimental
