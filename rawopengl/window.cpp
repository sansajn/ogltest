#include "window.hpp"
#include <algorithm>
#include <map>
#include <cassert>
#include <GL/freeglut.h>

namespace ui {

using std::map;
using std::make_pair;
using std::min;
using std::max;
using std::tuple;
using std::make_tuple;

basic_window::parameters::parameters()
{
	_name = "window";
	_version = std::make_pair(-1, -1);  // default version
	_debug = false;
	_w = 800;
	_h = 600;
}

namespace detail {

basic_window * active_window();
event_handler::key tospecial(int k);
event_handler::modifier tomodifier(int m);

void display_func();
void reshape_func(int w, int h);
void idle_func();
void close_func();
void mouse_func(int button, int state, int x, int y);
void motion_func(int x, int y);
void passive_motion_func(int x, int y);
void keyboard_func(unsigned char key, int x, int y);
void keyboard_up_func(unsigned char key, int x, int y);
void special_func(int key, int x, int y);
void special_up_func(int key, int x, int y);

map<int, basic_window *> glut_windows;

basic_glut_impl::basic_glut_impl(parameters const & p)
{
	if (glut_windows.empty())
	{
		int argc = 1;
		char * argv[1] = {(char *)"dummy"};
		glutInit(&argc, argv);
	}

	if (p.version() != make_pair(-1, -1))
		glutInitContextVersion(p.version().first, p.version().second);

	if (p.debug())
		glutInitContextFlags(GLUT_DEBUG);

	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	unsigned w = p.width();
	unsigned h = p.height();
	glutInitWindowSize(w, h);

	_wid = glutCreateWindow(p.name().c_str());
	glut_windows[_wid] = this;

	glutReshapeFunc(reshape_func);
	glutIdleFunc(idle_func);
	glutCloseFunc(close_func);
	glutMouseFunc(mouse_func);
	glutMotionFunc(motion_func);
	glutPassiveMotionFunc(passive_motion_func);
	glutKeyboardFunc(keyboard_func);
	glutKeyboardUpFunc(keyboard_up_func);
	glutSpecialFunc(special_func);
	glutSpecialUpFunc(special_up_func);
}

basic_glut_impl::~basic_glut_impl()
{
	if (glutGetWindow())  // ak som klikol na x (okno je uz neplatne)
		glutDestroyWindow(_wid);
	glut_windows.erase(_wid);
}

void basic_glut_impl::start()
{
	glutMainLoop();
}

void basic_glut_impl::display()
{
	glutSwapBuffers();
}

}  // detail


glut_event_impl::glut_event_impl(parameters const & p) : base(p)
{
	glutDisplayFunc(detail::display_func);
}


glut_pool_impl::glut_pool_impl(parameters const & p) : base(p), _fps(make_tuple(0.0f, 1e6f, 0.0f))
{}

glut_pool_impl::kbm_input::kbm_input()
{
	keyb_init();
	mouse_init();
}

void glut_pool_impl::kbm_input::update()
{
	keyb_update();
	mouse_update();
}

bool glut_pool_impl::kbm_input::key(unsigned char c) const
{
	return _keys[c];
}

bool glut_pool_impl::kbm_input::key_up(unsigned char c) const
{
	return _keys_up[c];
}

bool glut_pool_impl::kbm_input::mouse(button b) const
{
	return _mouse_buttons[int(b)];
}

bool glut_pool_impl::kbm_input::mouse_up(button b) const
{
	return _mouse_buttons_up[int(b)];
}

bool glut_pool_impl::kbm_input::wheel_up(wheel w) const
{
	return _mouse_buttons_up[w == wheel::up ? int(button::wheel_up) : int(button::wheel_down)];
}


void glut_pool_impl::start()
{
	unsigned t_last = glutGet(GLUT_ELAPSED_TIME);

	while (true)
	{
		unsigned t = glutGet(GLUT_ELAPSED_TIME);
		float dt = float(t - t_last)/1000.0f;  // in sec
		t_last = t;

		glutMainLoopEvent();
		if (_closed)
			break;

		input(dt);
		update(dt);
		display();

		in.update();
	}
}

void glut_pool_impl::update(float dt)
{
	static unsigned frames = 0;
	static float time_count = 0;

	frames += 1;
	time_count += dt;

	if (time_count > 1.0f)
	{
		float curr = frames/time_count;
		_fps = make_tuple(curr, min(std::get<1>(_fps), curr), max(std::get<2>(_fps), curr));
		frames = 0;
		time_count = 0.0f;
	}
}

void glut_pool_impl::close()
{
	_closed = true;
}

void glut_pool_impl::mouse_motion(int x, int y)
{
	in._mouse_pos = glm::ivec2(x,y);
}

void glut_pool_impl::mouse_passive_motion(int x, int y)
{
	in._mouse_pos = glm::ivec2(x,y);
}

void glut_pool_impl::mouse_click(button b, state s, modifier m, int x, int y)
{
	if (s == state::down)
		in._mouse_buttons[int(b)] = true;
	else
	{
		in._mouse_buttons[int(b)] = false;
		in._mouse_buttons_up[int(b)] = true;
	}

	in._mouse_pos = glm::ivec2(x,y);
}

void glut_pool_impl::mouse_wheel(wheel w, modifier m, int x, int y)
{
	if (w == wheel::up)
		in._mouse_buttons_up[int(button::wheel_up)] = true;

	if (w == wheel::down)
		in._mouse_buttons_up[int(button::wheel_down)] = true;
}

void glut_pool_impl::key_typed(unsigned char c, modifier m, int x, int y)
{
	in._keys[c] = true;
}

void glut_pool_impl::key_released(unsigned char c, modifier m, int x, int y)
{
	in._keys[c] = false;
	in._keys_up[c] = true;
}

void glut_pool_impl::kbm_input::keyb_init()
{
	for (int i = 0; i < NUM_KEYS; ++i)
		_keys[i] = _keys_up[i] = false;
}

void glut_pool_impl::kbm_input::mouse_init()
{
	_mouse_pos = glm::vec2(0,0);  // TODO: zvaz hodnotu center
	for (int i = 0; i < int(button::number_of_buttons); ++i)
		_mouse_buttons[i] = _mouse_buttons_up[i] = false;
}

void glut_pool_impl::kbm_input::keyb_update()
{
	for (bool & k : _keys_up)
		k = false;
}

void glut_pool_impl::kbm_input::mouse_update()
{
	for (bool & b : _mouse_buttons_up)
		b = false;
}


namespace detail {

void display_func()
{
	active_window()->display();
}

void reshape_func(int w, int h)
{
	active_window()->reshape(w, h);
}

void idle_func()
{
	active_window()->idle();
}

void close_func()
{
	active_window()->close();
}

void mouse_func(int mouse_btn, int btn_state, int x, int y)
{
	using eh = event_handler;

	basic_window * w = active_window();
	eh::modifier m = tomodifier(glutGetModifiers());

	if (mouse_btn == 3 || mouse_btn == 4)  // 3:button::wheel_up, 4:button::wheel_down
	{
		if (btn_state == GLUT_DOWN)
			w->mouse_wheel((mouse_btn == 3 ? eh::wheel::up : eh::wheel::down), m, x, y);
	}
	else
		w->mouse_click(eh::button(mouse_btn), eh::state(btn_state), m, x, y);
}

void motion_func(int x, int y)
{
	active_window()->mouse_motion(x, y);
}

void passive_motion_func(int x, int y)
{
	active_window()->mouse_passive_motion(x, y);
}

void keyboard_func(unsigned char c, int x, int y)
{
	event_handler::modifier m = tomodifier(glutGetModifiers());
	active_window()->key_typed(c, m, x, y);
}

void keyboard_up_func(unsigned char c, int x, int y)
{
	event_handler::modifier m = tomodifier(glutGetModifiers());
	active_window()->key_released(c, m, x, y);
}

void special_func(int k, int x, int y)
{
	using eh = event_handler;

	eh::key special = tospecial(k);
	if (special != eh::key::unknown)
	{
		eh::modifier m = tomodifier(glutGetModifiers());
		active_window()->special_key(special, m, x, y);
	}
}

void special_up_func(int k, int x, int y)
{
	using eh = event_handler;

	eh::key special = tospecial(k);
	if (special != eh::key::unknown)
	{
		eh::modifier m = tomodifier(glutGetModifiers());
		active_window()->special_key_released(special, m, x, y);
	}
}

basic_window * active_window()
{
	auto it = glut_windows.find(glutGetWindow());
	assert(it != glut_windows.end() && "unknown glut window");
	return it->second;
}

event_handler::key tospecial(int k)
{
	using eh = event_handler;

	switch (k)
	{
		case GLUT_KEY_LEFT:
			return eh::key::left;
		case GLUT_KEY_UP:
			return eh::key::up;
		case GLUT_KEY_RIGHT:
			return eh::key::right;
		case GLUT_KEY_DOWN:
			return eh::key::down;
		case GLUT_KEY_PAGE_UP:
			return eh::key::page_up;
		case GLUT_KEY_PAGE_DOWN:
			return eh::key::page_down;
		case GLUT_KEY_HOME:
			return eh::key::home;
		case GLUT_KEY_END:
			return eh::key::end;
		case GLUT_KEY_INSERT:
			return eh::key::insert;
	};

	if ((k > GLUT_KEY_F1-1) && k < (GLUT_KEY_F12+1))
		return eh::key(k-GLUT_KEY_F1+1);

	return eh::key::unknown;  // undocumented alt, ctrl and shift
}

event_handler::modifier tomodifier(int m)
{
	using eh = event_handler;

	int mods = eh::modifier::none;
	if (m & GLUT_ACTIVE_SHIFT)
		mods |= eh::modifier::shift;
	if (m & GLUT_ACTIVE_CTRL)
		mods |= eh::modifier::ctrl;
	if (m & GLUT_ACTIVE_ALT)
		mods |= eh::modifier::alt;

	return eh::modifier(mods);
}

void glew_init()
{
	assert(glGetError() == 0);
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
		throw window_error("GLEW initialization failed");
	glGetError();  // eat error
}

}  // detail

}  // ui

#include <pthread.h>
void junk()  // riesi bug v linuxe, ked pri spusteni binarky system zahlasi 'Inconsistency detected by ld.so: dl-version.c: 224: _dl_check_map_versions: Assertion `needed != ((void *)0)' failed!'
{
	int i;
	i=pthread_getconcurrency();
}
