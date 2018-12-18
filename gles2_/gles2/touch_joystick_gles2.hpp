#pragma once
#include <glm/matrix.hpp>
#include "gles2/mesh_gles2.hpp"
#include "gles2/program_gles2.hpp"

namespace gles2 { namespace ui { namespace touch {

class joystick  //!< implementacie joysticku pre touch-screen zariadenia
{
public:
	enum class touch_event {down, move, up};  // TODO: tento touch event treba zrusit

	joystick(glm::ivec2 const & origin, unsigned radius, unsigned screen_w, unsigned screen_h);  //!< origin and radius are in pixels (ortho projection)
	bool up() const;
	bool down() const;
	bool left() const;
	bool right() const;
	void render();
	void screen_geometry(unsigned width, unsigned height);
	void touch(glm::ivec2 const & pos, touch_event event);  //!< internal use only

private:
	void set_dirs(bool up, bool down, bool left, bool right);
	void calculate_projection(unsigned screen_w, unsigned screen_h);

	bool _dirs[4];  //!< up, down, left, right
	glm::ivec2 _origin;
	unsigned _radius;  //!< in pixels
	glm::ivec2 _inner_origin;
	glm::mat4 _proj;
	glm::mat4 _local_to_camera;
	gles2::mesh _circle;
	gles2::shader::program _solid;  // TODO: share!
};

}}}  // gles2::ui::touch
