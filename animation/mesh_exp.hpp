// experimental mesh implementation
#pragma once
#include <vector>

#include <glm/glm.hpp>
using std::vector;
using glm::vec3;
using glm::vec2;
using glm::vec4;
using glm::ivec4;


namespace gl {

struct attribute
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

	// debug
	struct dbg_vertex_buffer {
		vector<vec3> position;
		vector<vec2> texcoord;
		vector<vec3> normal;
		vector<ivec4> joints;
		vector <vec4> weights;
	};

	dbg_vertex_buffer __vertbuf;

private:
	enum {vbo_id, ibo_id, buffer_count};

	unsigned _nindices;
	std::vector<attribute> _attribs;
	unsigned _gpu_buffer_ids[buffer_count];
	int _draw_mode;  //!< \sa glDrawElements()
};

}  // gl
