#include "sfml_window.h"

//namespace gl {

//sfml_window::sfml_window(parameters const & params)
//	: _window(sf::VideoMode(params.width(), params.height()), params.name())
//{}

//void sfml_window::start()
//{
//	while (_window.IsOpened())
//	{
//		handle_events();
//		if (!_window.IsOpened())
//			break;
//		display();
//		idle();
//	}
//}

//void sfml_window::handle_events()
//{
//	sf::Event event;
//	while (_window.GetEvent(event))
//	{
//		switch (event.Type)
//		{
//			case sf::Event::Resized:
//			{
//				reshape(event.Size.Width, event.Size.Height);
//				break;
//			}
//		}  // switch

///*
//		handle_mouse(event);
//		handle_keyboard(event);
//*/
//	}
//}

//void sfml_window::handle_mouse(sf::Event & e)
//{
//	switch (e.Type)
//	{
//		case sf::Event::MouseMoved:
//			_mouse_pos = glm::vec2(e.MouseMove.x, e.MouseMove.y);
//			if (!_mouse_down)
//				mouse_passive_motion(e.MouseMove.x, e.MouseMove.y);
//			else
//				mouse_motion(e.MouseMove.x, e.MouseMove.y);
//			break;

//		case sf::Event::MouseButtonReleased:
//			_mouse_down = false;
//			mouse_click(tobutton(e.MouseButton.Button), state::up, e.MouseButton.x,
//				e.MouseButton.y);
//			break;

//		case sf::Event::MouseButtonPressed:
//			_mouse_down = true;
//			mouse_click(tobutton(e.MouseButton.Button), state::down,
//				e.MouseButton.x, e.MouseButton.y);
//			break;

//		case sf::Event::MouseWheelMoved:
//			mouse_wheel();
//			break;
//	}  // switcha
//}

//event_handler::button tobutton(sf::Mouse::Button & b)
//{
//	switch (b)
//	{
//		case sf::Mouse::Button::Left:
//			return event_handler::button::left;
//		case sf::Mouse::Button::Right:
//			return event_handler::button::right;
//		case sf::Mouse::Button::Middle:
//			return event_handler::button::middle;
//	}
//}

//bool isspecial(sf::Key::Code x)
//{
//	return false;
//}

//event_handler::key tokey(sf::Key::Code x)
//{
//}

//void sfml_window::handle_keyboard(sf::Event & e)
//{
//	switch (e.Type)
//	{
//		case sf::Event::KeyPressed:
//		{
//			if (isspecial(e.Key.Code))
//				special_key(tokey(e.Key.Code), _mouse_pos.x, _mouse_pos.y);
//			else
//				key_typed((unsigned char)e.Key.Code, _mouse_pos.x, _mouse_pos.y);
//			break;
//		}

//		case sf::Event::KeyReleased:
//		{
//			if (isspecial(e.Key.Code))
//				special_key_released(tokey(e.Key.Code), _mouse_pos.x, _mouse_pos.y);
//			else
//				key_released((unsigned char)e.Key.Code, _mouse_pos.x, _mouse_pos.y);
//			break;
//		}
//	}  // switch
//}


///*
//virtual void mouse_click(int button, int state, int x, int y) {}
//virtual void mouse_motion(int x, int y) {}
//virtual void mouse_passive_motion(int x, int y) {}
//virtual void mouse_wheel(int wheel, int direction, int x, int y) {}
//virtual void key_typed(unsigned char key, int x, int y) {}
//virtual void key_released(unsigned char key, int x, int y) {}
//virtual void special_key(int key, int x, int y) {}
//virtual void special_key_released(int key, int x, int y) {}
//*/

//}   // gl
