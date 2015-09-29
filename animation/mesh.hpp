#pragma once
#include <vector>
#include <string>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

class mesh
{
public:
	mesh();
	// vbuf je vec3:position, vec2:uv, vec3:normal, vec3:tangent
	mesh(std::vector<float> const & vbuf, std::vector<unsigned> const & ibuf);
	mesh(mesh && other);
	~mesh();
	void draw() const;  // render()
	void operator=(mesh && other);

	mesh(mesh const &) = delete;
	void operator=(mesh const &) = delete;

private:
	enum {vbo_idx = 0, ibo_idx = 1};

	unsigned _gpu_buffer_id[2];  // vbo, ibo
	unsigned _nindices;
};

mesh mesh_from_file(std::string const & fname, unsigned mesh_idx = 0);
mesh mesh_from_memory(void const * buf, unsigned len, char const * file_format);

class model
{
public:
	model() {}
	model(std::string const & fname);
	model(model && other);
	~model() {}
	void draw() const;
	void operator=(model && other);

	model(model const &) = delete;
	void operator=(model const &) = delete;

private:
	std::vector<mesh> _meshes;
};

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

mesh mesh_from_vertices(std::vector<vertex> const & verts, std::vector<unsigned> const & indices);


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
