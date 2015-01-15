#pragma once
#include <string>

class mesh
{
public:
	mesh(std::string const & fname);
	~mesh();

	void draw() const;

	void operator=(mesh const &) = delete;

private:
	unsigned _bufs[2];  // vbo, ibo
	unsigned _size;  // indices size
};
