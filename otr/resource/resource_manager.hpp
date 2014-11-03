#pragma once
#include <map>
#include <string>
#include <exception>
#include "core/ptr.hpp"
#include "resource.hpp"

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

private:
	std::map<std::string, ptr<resource>> _resources;
};

template <typename R>
ptr<R> resource_manager::load_resource(std::string const & name)
{
	return std::dynamic_pointer_cast<R>(load_resource(name));
}
