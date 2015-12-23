#include "glut_window.hpp"
#include <utility>

namespace ui {

using std::make_pair;
using glm::ivec2;

namespace detail {

void glew_init();

}  // detail

namespace glut_detail {

glut_layer * active_window();
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

glut_layer * __window = nullptr;

}  // glut_detail


glut_layer::glut_layer(parameters const & params)
{
	if (!glut_detail::__window)
	{
		int argc = 1;
		char const * argv = "dummy";
		glutInit(&argc, (char **)&argv);
	}

	if (params.version() != make_pair(0, 0))
		glutInitContextVersion(params.version().first, params.version().second);

	if (params.debug())
		glutInitContextFlags(GLUT_DEBUG);

	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutInitWindowSize(params.width(), params.height());

	glutCreateWindow(params.name().c_str());
	glut_detail::__window = this;

	glutReshapeFunc(glut_detail::reshape_func);
	glutCloseFunc(glut_detail::close_func);
	glutMouseFunc(glut_detail::mouse_func);
	glutMotionFunc(glut_detail::motion_func);
	glutPassiveMotionFunc(glut_detail::passive_motion_func);
	glutKeyboardFunc(glut_detail::keyboard_func);
	glutKeyboardUpFunc(glut_detail::keyboard_up_func);
	glutSpecialFunc(glut_detail::special_func);
	glutSpecialUpFunc(glut_detail::special_up_func);

	detail::glew_init();
}

glut_layer::~glut_layer()
{
	int wid = glutGetWindow();
	if (wid)  // ak som klikol na x (okno je uz neplatne)
		glutDestroyWindow(wid);
	glut_detail::__window = nullptr;
}

void glut_layer::install_display_handler()
{
	glutDisplayFunc(glut_detail::display_func);
}

glut_layer::user_input::user_input()
{
	for (unsigned i = 0; i < NUM_KEYS; ++i)
		_keys[i] = _keys_up[i] = false;

	_mouse_pos = glm::vec2(0,0);  // TODO: zvaz hodnotu center
	for (unsigned i = 0; i < NUM_BUTTONS; ++i)
		_mouse_buttons[i] = _mouse_buttons_up[i] = false;
}

bool glut_layer::user_input::key(unsigned char c) const
{
	return _keys[c];
}

bool glut_layer::user_input::key_up(unsigned char c) const
{
	return _keys_up[c];
}

bool glut_layer::user_input::any_of_key(char const * s) const
{
	while (*s)
		if (key(*s++))
			return true;
	return false;
}

bool glut_layer::user_input::any_of_key_up(char const * s) const
{
	while (*s)
		if (key_up(*s++))
			return true;
	return false;
}

bool glut_layer::user_input::mouse(button b) const
{
	return _mouse_buttons[(int)b];
}

bool glut_layer::user_input::mouse_up(button b) const
{
	return _mouse_buttons_up[(int)b];
}

bool glut_layer::user_input::mouse_wheel(wheel w) const
{
	return _mouse_buttons_up[w == wheel::up ? (int)button::wheel_up : (int)button::wheel_down];
}

ivec2 const & glut_layer::user_input::mouse_position() const
{
	return _mouse_pos;
}

void glut_layer::user_input::update()
{
	for (bool & k : _keys_up)  // set all keys up to false
		k = false;

	for (bool & b : _mouse_buttons_up)  // set all buttons up to false
		b = false;
}


namespace glut_detail {

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

	glut_layer * w = active_window();
	eh::modifier m = tomodifier(w->modifiers());

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

glut_layer * active_window()
{
	return __window;
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

}  // glut_detail


namespace detail {

void glew_init()
{
	assert(glGetError() == 0);
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
		throw std::runtime_error("GLEW initialization failed");
	glGetError();  // eat error
}

}  // detail

}  // ui
