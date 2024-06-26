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

	vertex(glm::vec3 const & position, glm::vec2 const & uv) : position(position), uv(uv), normal(0,0,0), tangent(0,0,1) {}
};

class mesh
{
public:
	mesh();
	mesh(std::string const & fname);
	mesh(std::vector<vertex> const & verts, std::vector<unsigned> const & indices);
	mesh(mesh && lhs);
	~mesh();

	void open(std::string const & fname);
	void create(std::vector<vertex> const & verts, std::vector<unsigned> const & indices);
	void draw() const;
	void free();

	void operator=(mesh && lhs);

	mesh(mesh const &) = delete;
	void operator=(mesh const &) = delete;

private:
	unsigned _bufs[2];  // vbo, ibo
	unsigned _size;  // number of indices
};
