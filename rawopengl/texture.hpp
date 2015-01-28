#pragma once
#include <string>

class texture
{
public:
	texture(std::string const & fname);
	~texture();
	void bind(unsigned unit);

private:
	unsigned _tid;
};
