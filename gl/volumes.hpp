#pragma once
#include "mesh.hpp"

gl::mesh make_cube();  //!< vytvori unit cube s taziskom na pozicii (0,0,0)
gl::mesh make_box(glm::vec3 const & half_extents);  //!< vytvori obdlznik s taziskom v (0,0,0) s polstranou velkosti half_extents
