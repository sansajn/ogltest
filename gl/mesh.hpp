#pragma once
#include <vector>
#include <string>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

struct vertex
{
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec3 tangent;

	vertex() {}
	vertex(glm::vec3 const & position, glm::vec2 const & uv) : position(position), uv(uv), normal(0,0,0), tangent(0,0,1) {}
	vertex(glm::vec3 const & position, glm::vec2 const & uv, glm::vec3 const & normal) : position(position), uv(uv), normal(normal), tangent(0,0,1) {}
};

class mesh
{
public:
	mesh();
	mesh(std::string const & fname);
	mesh(std::vector<vertex> const & verts, std::vector<unsigned> const & indices);
	mesh(mesh && lhs);
	~mesh();

	void from_file(std::string const & fname);
	void from_memory(void const * buf, unsigned len, char const * format);
	void from_vertices(std::vector<vertex> const & verts, std::vector<unsigned> const & indices);
	void draw() const;
	void free();

	void operator=(mesh && lhs);

	mesh(mesh const &) = delete;
	void operator=(mesh const &) = delete;

private:
	unsigned _bufs[2];  // vbo, ibo
	unsigned _size;  // number of indices
};

// utils
mesh make_quad_xy();  //!< (-1,-1), (1,1)
mesh make_unit_quad_xy();  //!< (0,0), (1,1)
mesh make_quad_xy(glm::vec2 const & origin, float size);
mesh make_quad_xz();
mesh make_quad_xz(glm::vec2 const & origin, float size);

mesh make_plane_xy(unsigned w, unsigned h);  //!< stred roviny je v lavom dolnom rohu
mesh make_plane_xy(glm::vec3 const & origin, float size, unsigned w, unsigned h);
mesh make_plane_xz(unsigned w, unsigned h);

mesh make_cube();  //!< unit cube (stred kocky je v 0,0,0 zo stranou velkou 1)
mesh make_cube(glm::vec3 const & position, float size);

mesh make_sphere();  //!< unit sphere
