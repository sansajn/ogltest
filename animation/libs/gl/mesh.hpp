// experimental mesh implementation
#pragma once
#include <vector>
#include <string>
#include <memory>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

namespace gl {

struct attribute  // buffer attribute
{
	unsigned index;  //!< attribute index from shader program
	int size;  //!< number of attribute elements (4 for vec4, 3 for vec3, ...)
	int type;  //!< GL_FLOAT, ...
	int normalized;  //!< GL_FALSE or GL_TRUE
	unsigned stride;
	int start_idx; //!< index where the data starts
	bool int_type;  //!< true if type is an integer type

	attribute(unsigned index, int size, int type, unsigned stride, int start_idx = 0, int normalized = 0);
};

class mesh
{
public:
	mesh();
	mesh(void const * vbuf, unsigned vbuf_sizeof, unsigned const * ibuf, unsigned ibuf_size);
	mesh(mesh && other);
	~mesh();
	void render() const;
	void append_attribute(attribute const & a);
	void draw_mode(int mode);  //!< \sa glDrawElements()
	void operator=(mesh && other);

	mesh(mesh const &) = delete;
	void operator=(mesh const &) = delete;

private:
	enum {vbo_id, ibo_id, buffer_count};

	unsigned _nindices;
	std::vector<attribute> _attribs;
	unsigned _gpu_buffer_ids[buffer_count];
	int _draw_mode;  //!< \sa glDrawElements()
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

mesh mesh_from_file(std::string const & fname, unsigned mesh_idx = 0);
mesh mesh_from_memory(void const * buf, unsigned len, char const * file_format);
mesh mesh_from_vertices(std::vector<vertex> const & verts, std::vector<unsigned> const & indices);

class model
{
public:
	model() {}
	model(model && other);
	virtual ~model() {}
	virtual void render() const;
	void append_mesh(std::shared_ptr<mesh> m);
	void operator=(model && other);

	model(model const &) = delete;
	void operator=(model const &) = delete;

private:
	std::vector<std::shared_ptr<mesh>> _meshes;
};

model model_from_file(std::string const & fname);

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

}  // gl
