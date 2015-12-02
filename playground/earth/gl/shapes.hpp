#pragma once
#include "gl/mesh.hpp"

namespace gl {

mesh make_cube();  //!< vytvori unit cube zo stredom na pozicii (0,0,0)
mesh make_box(glm::vec3 const & half_extents);  //!< vytvori obdlznik zo stredom v (0,0,0) s polstranou velkosti half_extents
mesh make_sphere(float r = 1.0f, unsigned hsegments = 30, unsigned vsegments = 20);  //!< vytvori gulu zo stredom v (0,0,0)
mesh make_cylinder(float r, float h, unsigned segments = 20);  //!< vytvori cylinder zo stredom v (0,0,0)
mesh make_cone(float r, float h, unsigned segments = 20);  //!< vytvori kuzel zo stredom v (0,0,0)
mesh make_disk(float r, unsigned segments = 20);  //!< vytvori disk zo stredom v (0,0,0) s polomerom r
mesh make_open_cylinder(float r, float h, unsigned segments = 20);  //!< vytvori zhora, zdola otvoreny cylinder zo stredom v (0,0,0)
// TODO: make_plane(), make_plane_xz(), make_quad(), make_quad_xz()

// utils
mesh make_quad_xy();  //!< (-1,-1), (1,1)
mesh make_unit_quad_xy();  //!< (0,0), (1,1)
mesh make_quad_xy(glm::vec2 const & origin, float size);
mesh make_quad_xz();
mesh make_quad_xz(glm::vec2 const & origin, float size);
mesh make_quad_zy();
mesh make_quad_zy(glm::vec2 const & origin, float size);

// pociatok roviny je v lavom dolnom rohu, w a h je pocet vrcholov roviny v dannom smere
mesh make_plane_xy(unsigned w, unsigned h);
mesh make_plane_xy(glm::vec3 const & origin, float size, unsigned w, unsigned h);
mesh make_plane_xz(glm::vec3 const & origin = glm::vec3{0,0,0}, float size = 1.0f, unsigned w = 11, unsigned h = 11);
mesh make_plane_xz(unsigned w, unsigned h, float size = 1.0f);  // TODO: odstran

mesh make_axis();

}  // gl
