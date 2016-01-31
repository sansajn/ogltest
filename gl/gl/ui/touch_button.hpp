#pragma once
#include <memory>
#include <glm/matrix.hpp>
#include "gl/mesh.hpp"
#include "gl/program.hpp"
#include "gl/ui/touch_event.hpp"

namespace ui { namespace touch {

class button  //!< tlacitko pre touch-screen
{
public:
	button() {}
	void init(glm::ivec2 const & origin, unsigned radius, unsigned screen_w, unsigned screen_h, std::shared_ptr<shader::program> prog);  // TODO: implementuj move semantiku
	bool up() const;
	bool down() const;
	void update();
	void render();
	void screen_geometry(unsigned width, unsigned height);
	bool touch(glm::ivec2 const & pos, touch_event event);  //!< internal use only

private:
	glm::ivec2 _origin;
	unsigned _radius;
	bool _up, _down;
	gl::mesh _circle;
	glm::mat4 _local_to_screen;
	std::shared_ptr<shader::program> _prog;  // expect solid shader program
};

}}  // ui::touch
