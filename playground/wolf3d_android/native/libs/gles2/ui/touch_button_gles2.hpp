#pragma once
#include <memory>
#include <glm/matrix.hpp>
#include "gles2/mesh_gles2.hpp"
#include "gles2/program_gles2.hpp"
#include "gles2/ui/touch_event_gles2.hpp"
#include "gl/window.hpp"

namespace ui { namespace touch {

class button  //!< tlacitko pre touch-screen
{
public:
	button() {}
	void init(glm::ivec2 const & origin, unsigned radius, unsigned screen_w, unsigned screen_h, std::shared_ptr<gles2::shader::program> prog);  // TODO: implementuj move semantiku
	bool up() const;
	bool down() const;
	void update();
	void render();
	void screen_geometry(unsigned width, unsigned height);

	template <typename UserInput>
	bool input(UserInput & in, float dt);

private:
	bool evaluate_finger(ui::touch::finger const & f);

	glm::ivec2 _origin;
	unsigned _radius;
	bool _up, _down;
	int _down_finger_id;
	gles2::mesh _circle;
	glm::mat4 _local_to_screen;
	std::shared_ptr<gles2::shader::program> _prog;  // expect solid shader program
};


template <typename UserInput>
bool button::input(UserInput & in, float dt)
{
	for (ui::touch::finger & f : in.touch.fingers())
		if (evaluate_finger(f))
			return true;
	return false;
}

}}  // ui::touch
