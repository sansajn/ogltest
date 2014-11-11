#pragma once
#include <map>
#include <string>
#include "core/ptr.hpp"
#include "resource.hpp"

class resource_manager;

class resource_factory
{
public:
	using create_func = ptr<resource> (*)(resource_descriptor *, resource_manager *);

	static resource_factory & ref();
	ptr<resource> create(resource_descriptor * desc, resource_manager * resman);

	template <typename T>
	struct type  // automaticka registracia zdrojou
	{
		static ptr<resource> ctor(resource_descriptor * desc, resource_manager * resman)
		{
			return make_ptr<T>(desc, resman);
		}

		type(std::string const & name) {resource_factory::ref().append_type(name, type<T>::ctor);}
	};

private:
	void append_type(std::string const & name, create_func f);

	std::map<std::string, create_func> _creators;
};
