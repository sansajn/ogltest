#pragma once
#include <glm/matrix.hpp>
#include "gl/mesh.hpp"
#include "gl/program.hpp"

namespace ui {
	namespace touch {

class joystick  //!< implementacie joysticku pre touch-screen zariadenia
{
public:
	enum class touch_event {down, move, up};

	joystick(glm::ivec2 const & origin, unsigned radius, unsigned screen_w, unsigned screen_h);  //!< origin and radius are in pixels (ortho projection)
	bool up() const;
	bool down() const;
	bool left() const;
	bool right() const;
	void render();
	void screen_geometry(unsigned width, unsigned height);
	bool touch(glm::ivec2 const & pos, touch_event event);  //!< internal use only

private:
	void set_dirs(bool up, bool down, bool left, bool right);
	void calculate_projection(unsigned screen_w, unsigned screen_h);

	bool _dirs[4];  //!< up, down, left, right
	glm::ivec2 _origin;
	unsigned _radius;  //!< in pixels
	glm::ivec2 _inner_origin;
	glm::mat4 _proj;
	glm::mat4 _local_to_camera;
	gl::mesh _circle;
	shader::program _solid;  // TODO: share!
};

	}  // touch
}  // ui
