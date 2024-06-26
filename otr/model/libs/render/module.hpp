#pragma once
#include <map>
#include <set>
#include <string>
#include "core/ptr.hpp"
#include "any_value.hpp"

class program;

/*! Implementuje shader program modul (program sa skladá s viacerích modulou a modul s viacerých shader object-ov).
\ingroup render */
class module
{
public:
	module(int version, char const * vertex, char const * fragment);
	~module();
	int vertex_shader_id() const {return _vertex_shader_id;}
	int fragment_shader_id() const {return _fragment_shader_id;}
	std::set<program *> const & users() const {return _users;}
	void initial_value(ptr<any_value> value);

protected:
	module() {}  //!< vytvorí neinicializovaný modul
	void init(int version, char const * vertex, char const * fragment);

private:
	bool check(int shader_id);
	void print_log(int shader_id, int nlines, char const ** lines, bool error);

	std::set<program *> _users;
	int _vertex_shader_id;
	int _fragment_shader_id;
	std::map<std::string, ptr<any_value>> _initial_values;

	friend class program;
};
