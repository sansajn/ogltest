#include "tweakbar_resource.hpp"
#include <sstream>
#include "luatools/luatools.hpp"
#include "luatools/error_output.hpp"
#include "luatools/table.hpp"
#include "render/uniform.hpp"
#include "scenegraph/scene.hpp"
#include "core/logger.hpp"
#include "render/types.hpp"

using std::string;
using std::ostringstream;

static twbar_variable_desc bool_element(lua_State * L);
static twbar_variable_desc float_element(lua_State * L);
static string build_definition_string(twbar_variable_desc const & var);

// TODO: ten type odvod od TW_TYPE_XXX
class uniform_data : public tweakbar_resource::data
{
public:
	uniform_data(uniform_type type, std::string const & path, resource_manager * resman)
		: _type(type), _path(path), _resman(resman)
	{
		parse_path(path);
	}

	void get(void * value);
	void set(void const * value);

private:
	ptr<uniform> get_uniform();
	void parse_path(std::string const & path);

	uniform_type _type;
	std::string _path, _node_name, _module_name, _uniform_name;  // path:'sky/material/hdr_exposure'
	resource_manager * _resman;
};

void uniform_data::get(void * value)
{
	ptr<uniform> u = get_uniform();
	if (!u)
		return;

	switch (_type)
	{
		case uniform_type::vec1f:
		{
			ptr<uniform1f> u1f = std::dynamic_pointer_cast<uniform1f>(u);
			assert(u1f && "bad_cast");
			if (u1f)
				*(float *)value = u1f->get();
			break;
		}

		case uniform_type::vec2f:
		{
			ptr<uniform2f> u2f = std::dynamic_pointer_cast<uniform2f>(u);
			assert(u2f && "bad_cast");
			if (u2f)
				*(glm::vec2 *)value = u2f->get();
			break;
		}

		case uniform_type::vec3f:
		{
			ptr<uniform3f> u3f = std::dynamic_pointer_cast<uniform3f>(u);
			assert(u3f && "bad_cast");
			if (u3f)
				*(glm::vec3 *)value = u3f->get();
			break;
		}

		case uniform_type::vec4f:
		{
			ptr<uniform4f> u4f = std::dynamic_pointer_cast<uniform4f>(u);
			assert(u4f && "bad_cast");
			if (u4f)
				*(glm::vec4 *)value = u4f->get();
			break;
		}

		default:
			assert(false && "uniform_type not supported");
	}
}

void uniform_data::set(void const * value)
{
	ptr<uniform> u = get_uniform();
	if (!u)
		return;

	switch (_type)
	{
		case uniform_type::vec1f:
		{
			ptr<uniform1f> u1f = std::dynamic_pointer_cast<uniform1f>(u);
			assert(u1f && "bad_cast");
			if (u1f)
				u1f->set(*(float *)value);
			break;
		}

		case uniform_type::vec2f:
		{
			ptr<uniform2f> u2f = std::dynamic_pointer_cast<uniform2f>(u);
			assert(u2f && "bad_cast");
			if (u2f)
				u2f->set(*(glm::vec2 *)value);
			break;
		}

		case uniform_type::vec3f:
		{
			ptr<uniform3f> u3f = std::dynamic_pointer_cast<uniform3f>(u);
			assert(u3f && "bad_cast");
			if (u3f)
				u3f->set(*(glm::vec3 *)value);
			break;
		}

		case uniform_type::vec4f:
		{
			ptr<uniform4f> u4f = std::dynamic_pointer_cast<uniform4f>(u);
			assert(u4f && "bad_cast");
			if (u4f)
				u4f->set(*(glm::vec4 *)value);
			break;
		}

		default:
			assert(false && "uniform_type not supported");
	}
}

ptr<uniform> uniform_data::get_uniform()
{
	ptr<uniform> u;
	ptr<scene_node> scene = _resman->load_resource<scene_node>("scene");
	for (ptr<scene_node> ch : scene->children())
	{
		if (ch->name() == _node_name)
		{
			ptr<shader::module> m = ch->get_module(_module_name);
			if (m)
			{
				if (!m->users().empty())
				{
					shader::program * p = *m->users().begin();
					u = p->get_uniform(_uniform_name);
					break;
				}
			}  // if m
		}  // if (ch->name()
	}  // for

	if (!u)
		wlog("TWEAKBAR") << "uniform '" << _path << "' not found";

	return u;
}

void uniform_data::parse_path(std::string const & path)
{
	std::string::size_type p1 = path.find('/');
	std::string::size_type p2 = path.find('/', p1+1);
	if (p1 == std::string::npos || p2 == std::string::npos)
		throw std::exception();  // TODO: specify (wrong path format)
	_node_name = path.substr(0, p1);
	_module_name = path.substr(p1+1, p2-p1-1);
	_uniform_name = path.substr(p2+1);
}


void TW_CALL set_uniform_callback(void const * value, void * client_data)
{
	uniform_data * d = (uniform_data *)client_data;
	d->set(value);
}

void TW_CALL get_uniform_callback(void * value, void * client_data)
{
	uniform_data * d = (uniform_data *)client_data;
	d->get(value);
}

tweakbar_resource::tweakbar_resource(std::vector<twbar_variable_desc> const & vars, resource_manager * resman)
	: tweakbar_handler("dummy", true), _vars(vars), _resman(resman)
{}

void tweakbar_resource::init(std::vector<twbar_variable_desc> const & vars, resource_manager * resman)
{
	_vars = vars;
	_resman = resman;
}

void tweakbar_resource::update_bar(TwBar * bar)
{
	clear_data();

	for (twbar_variable_desc const & var : _vars)
	{
		string def = build_definition_string(var);

		switch (var.type)
		{
			case TW_TYPE_BOOLCPP:
			{
				auto d = new uniform_data(uniform_type::vec1f, var.path, _resman);
				_datas.push_back(d);
				TwAddVarCB(bar, var.label.c_str(), TW_TYPE_BOOLCPP, set_uniform_callback, get_uniform_callback, d, def.c_str());
				break;
			}

			case TW_TYPE_FLOAT:
			{
				auto d = new uniform_data(uniform_type::vec1f, var.path, _resman);
				_datas.push_back(d);
				TwAddVarCB(bar, var.label.c_str(), TW_TYPE_FLOAT, set_uniform_callback, get_uniform_callback, d, def.c_str());
				break;
			}

			default:
				assert(false && "type is not supported");  // TODO: suport others
				break;
		}
	}  // for
}

void tweakbar_resource::clear_data()
{
	for (auto d : _datas)
		delete d;
	_datas.clear();
}

string build_definition_string(twbar_variable_desc const & var)
{
	ostringstream ostr;
	if (!var.label.empty())
		ostr << "label='" << var.label << "' ";
	if (!var.group.empty())
		ostr << "group='" << var.group << "' ";
	if (!var.help.empty())
		ostr << "help='" << var.help << "' ";
	if (!var.keyincr.empty())
		ostr << "keyincr='" << var.keyincr << "' ";
	if (!var.keydecr.empty())
		ostr << "keydecr='" << var.keydecr << "' ";
	if (var.min < std::numeric_limits<float>::max())
		ostr << "min=" << var.min << " ";
	if (var.max < std::numeric_limits<float>::max())
		ostr << "max=" << var.max << " ";
	if (var.step < std::numeric_limits<float>::max())
		ostr << "step=" << var.step;
	return ostr.str();
}
