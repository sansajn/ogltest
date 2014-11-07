#pragma once
#include <map>
#include <string>
#include <exception>
#include <cassert>
#include "core/ptr.hpp"


class resource
{
public:
	virtual ~resource() {}
};

/*! \ingroup resouce */
class resource_manager
{
public:
	resource_manager() {}
	~resource_manager() {}

	ptr<resource> load_resource(std::string const & name);

	template <typename R>
	ptr<R> load_resource(std::string const & name);

	// TODO: dočasne metody umožnujúce do managera vkladať objekty
	void insert_resource(std::string const & name, ptr<resource> r);

	std::string const & find_key(ptr<resource> r);  // TODO: pomale (potrebne pokial resource nebude mat meno)

private:
	std::map<std::string, ptr<resource>> _resources;
};

template <typename R>
ptr<R> resource_manager::load_resource(std::string const & name)
{
	ptr<R> r = std::dynamic_pointer_cast<R>(load_resource(name));
	assert(r && "cast failed");
	return r;
}
