#pragma once
#include <string>

/*! \ingroup resource */
class resource
{
public:
	resource(std::string const & name) : _name(name) {}
	virtual ~resource() {}
	std::string const & name() const {return _name;}

private:
	std::string _name;
};
