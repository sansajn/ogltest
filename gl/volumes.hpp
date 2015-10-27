#pragma once
#include "mesh.hpp"

gl::mesh make_cube();  //!< vytvori unit cube s taziskom na pozicii (0,0,0)
gl::mesh make_box(glm::vec3 const & half_extents);  //!< vytvori obdlznik s taziskom v (0,0,0) s polstranou velkosti half_extents
gl::mesh make_cylinder(float r, float h, unsigned segments = 20);  //!< vytvori cylinder zo stredom v (0,0,0)
gl::mesh make_disk(float r, unsigned segments = 20);  //!< vytvori disk zo stredom v (0,0,0) s polomerom r
gl::mesh make_open_cylinder(float r, float h, unsigned segments = 20);  //!< vytvori zhora, zdola otvoreny cylinder zo stredom v (0,0,0)
