#pragma once
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/quaternion.hpp>


namespace gl {

/*! Camera class implementation.
\code
	camera c{radians(70.0f), w/(float)h, 0.01f, 1000.0f};  // #1
	camera c{perspective(radians(70.0f), w/(float)h, 0.01f, 1000.0f)};  // #2
	camera c{ortho(0.0f, (float)w, (float)h, 0.0f, 0.01f, 1000.0f)};  // #3
\endcode

kde #1, standardne pouzitie kamery, #2 a #3 pouzitie kamery s uzivatelsky definovanou
projekcnou transformaciou. */
class camera
{
public:
	glm::vec3 position;
	glm::quat rotation;

	camera() {}
	camera(glm::mat4 const & P);
	camera(float fovy, float aspect, float near, float far);
	camera(glm::vec3 const & pos, float fovy, float aspect, float near, float far);

	void look_at(glm::vec3 const & center);

	glm::mat4 view() const;
	glm::mat4 const & projection() const {return _proj;}
	glm::mat4 view_projection() const {return _proj * view();}

	// view, projection and view_projection alternatives
	glm::mat4 world_to_camera() const {return view();}
	glm::mat4 const & camera_to_screen() const {return projection();}
	glm::mat4 world_to_screen() const {return view_projection();}

	glm::vec3 right() const;
	glm::vec3 up() const;
	glm::vec3 forward() const;  // TODO: forward je otoceny dozadu tzn.je to vlastne backward

	void projection(float fovy, float aspect, float near = 0.1f, float far = 1000.0f);
	void projection(glm::mat4 const & proj);

private:
	glm::mat4 _proj;  // _P koliduje s makrami v ctype.h v android clib
};

}  // gl
