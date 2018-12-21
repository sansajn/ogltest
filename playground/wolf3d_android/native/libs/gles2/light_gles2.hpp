#pragma once
#include <glm/vec3.hpp>
#include "gles2/program_gles2.hpp"

namespace gles2 {

/*! phong light shader helper
\code
_light = phong_light{normalize(vec3{2,3,5}), rgb::white, .85f};
// ...
_prog.use();
_light.apply(_prog);
\endcode
TODO: ako toto upravit, aby som mohol pouzit uniform namiesto neustaleho lookupu ? */
class phong_light
{
public:
	phong_light() {}
	phong_light(glm::vec3 const & direction, glm::vec3 const & color = glm::vec3{1}, float intensity = 1);
	void apply(shader::program & p);

	glm::vec3 direction;
	glm::vec3 color;
	float intensity;
};

class diffuse_light
{
public:
	diffuse_light() {}
	diffuse_light(glm::vec3 const & direction);
	void apply(shader::program & p);

	glm::vec3 direction;
};

}  // gl
