#pragma once
#include <list>
#include <glm/vec2.hpp>
#include "gl/window.hpp"

namespace ui {

struct touch_event
{
	glm::ivec2 position;
	event_handler::action finger_action;
	int finger_id;
};

class touch_list
{
public:
	struct finger
	{
		enum {down = 1, up = 2, move = 4, cancel = 8};  // finger states
		glm::ivec2 position;
		int id;
		int state;  //!< \note kombinacia hodnot down, move, up a cancel
	};

	using iterator = std::list<finger>::iterator;

	void insert(touch_event const & te);
	iterator erase(iterator pos) {return _fingers.erase(pos);}
	size_t size() const {return _fingers.size();}
	iterator begin() {return _fingers.begin();}
	iterator end() {return _fingers.end();}

private:
	std::list<finger> _fingers;
};

class touch_input
{
public:
	touch_list & fingers() {return _touches;}

	void update();
	void touch_performed(int x, int y, int finger_id, event_handler::action a);  //!< internl use only (vola okenna vrstva)

private:
	touch_list _touches;
};

class android_layer : public window_layer
{
public:
	using parameters = window_layer::parameters;

	android_layer(parameters const & params);
	~android_layer();

	class user_input  //!< keyboard, mouse and touch user input
	{
	public:
		user_input() {}

		touch_input touch;  // touch api

		void update();  //!< for internal use only

		// funkcie informujuce o zmene stavu uzivatelskeho vstupu (vola ich okenna vrstva)
		void mouse_motion(int x, int y) {}
		void mouse_passive_motion(int x, int y) {}
		void mouse_click(event_handler::button b, event_handler::state s, event_handler::modifier m, int x, int y) {}
		void mouse_wheel(event_handler::wheel w, event_handler::modifier m, int x, int y) {}
		void key_typed(unsigned char c, event_handler::modifier m, int x, int y) {}
		void key_released(unsigned char c, event_handler::modifier m, int x, int y) {}
		void touch_performed(int x, int y, int finger_id, event_handler::action a);
	};
};

using android_window = window<pool_behaviour, android_layer>;

}  // ui

// scene api
void create(int width, int height);  //!< volane pri vytvoreni egl kontextu (niekolko krat za zivot aplykacie)
void destroy();  //!< volane pri zniceni egl kontextu
