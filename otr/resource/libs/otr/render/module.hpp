#pragma once
#include <map>
#include <set>
#include <string>
#include "core/ptr.hpp"
#include "render/any_value.hpp"

namespace shader {

class program;

/*! Implementuje shader program modul (program sa skladá s viacerích modulou a modul s viacerých shader object-ov).
\ingroup render */
class module
{
public:
	module(int version, char const * src) {init(version, src);}
	module(int version, char const * vertex, char const * fragment) {init(version, vertex, fragment);}
	virtual ~module();
	int vertex_shader_id() const {return _vertex_shader_id;}
	int fragment_shader_id() const {return _fragment_shader_id;}
	std::set<program *> const & users() const {return _users;}  // TODO: raw pointer
	void initial_value(ptr<any_value> value);

protected:
	module() : _vertex_shader_id(-1), _fragment_shader_id(-1) {}
	void init(int version, char const * source);
	void init(int version, char const * vertex, char const * fragment);
	void init(int version, char const * vertex_header, char const * vertex, char const * fragment_header, char const * fragment);

private:
	bool check(int shader_id);
	void print_log(int shader_id, int nlines, char const ** lines, bool error);

	std::set<program *> _users;
	int _vertex_shader_id;
	int _fragment_shader_id;
	std::map<std::string, ptr<any_value>> _initial_values;

	friend program;  //!< modifikuje _users
};

}  // shader
