#pragma once
#include <map>
#include <string>
#include <exception>
#include <cassert>
#include <boost/any.hpp>
#include "core/ptr.hpp"


/*! \ingroup resouce */
class resource_manager
{
public:
	resource_manager() {}
	~resource_manager() {}

	boost::any & load_resource(std::string const & name);

	template <typename R>
	ptr<R> load_resource(std::string const & name);

	// TODO: dočasne metody umožnujúce do managera vkladať objekty
	template <typename T>
	void insert_resource(std::string const & name, ptr<T> r);

private:
	std::map<std::string, boost::any> _resources;
};

template <typename R>
ptr<R> resource_manager::load_resource(std::string const & name)
{
	try
	{
		return boost::any_cast<ptr<R>>(load_resource(name));
	}
	catch (boost::bad_any_cast &)
	{
		return ptr<R>();
	}
}

template <typename T>
void resource_manager::insert_resource(std::string const & name, ptr<T> r)
{
	boost::any a = r;
	auto result = _resources.insert(std::make_pair(name, a));
	assert(result.second && "resource with this name already in a manager");
}
