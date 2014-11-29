#pragma once
#include <string>

/*! Reprezentuje zdroj (textúra, mriežka, metóda, ...).
\saa resource_manager */
struct resource
{
	virtual ~resource() {}
};

/*! Trieda popisujúca zdroj.
\saa resource_loader */
struct resource_descriptor
{
public:
	std::string type;  //!< node, callmethod, foreach, ...
	resource_descriptor(std::string const & type) : type(type) {}
	virtual ~resource_descriptor() {}
};

/*! Pomocná šablóna transformujúca typ T na zdroj (resource). */
template <typename T>
class resource_template : public T, public resource
{
public:
	virtual ~resource_template() {}
};
