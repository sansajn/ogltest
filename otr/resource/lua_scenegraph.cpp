#include "lua_scenegraph.hpp"
#include <cassert>
#include "luatools/luatools.hpp"
#include "resource_manager.hpp"
#include "resource_factory.hpp"
#include "lua_resource.hpp"

//! Vrati nazov premennej na ktoru sa odkazuje premenna na vrchole zasobnika (tabulka).
std::string refers_to_gloabal_variable(lua_State * L)
{
	assert(lua_istable(L, -1) && "table expected");

	lua_getglobal(L, "_G");  // maps global variables
	std::map<ptrdiff_t, std::string> variables;
	for (lua::table_range r(L); r; ++r)
	{
		if (r->key_type() == LUA_TSTRING)
			variables[ptrdiff_t(lua_topointer(L, -1))] = r->key();
	}
	lua_pop(L, 1);

	auto it = variables.find(ptrdiff_t(lua_topointer(L, -1)));
	assert(it != variables.end() && "no such global variable");
	return it->second;
}

void table_field(lua_State * L, char const * name)
{
	assert(lua_istable(L, -1) && "table expected");
	lua_pushstring(L, name);
	lua_gettable(L, -2);
	assert(!lua_isnil(L, -1) && "such a field doesn't exist inside a table");
}

template <typename R>
R table_field(lua_State * L, const char * name)
{
	assert(lua_istable(L, -1) && "table expected");
	lua_pushstring(L, name);
	lua_gettable(L, -2);
	assert(lua::istype<R>(L) && "unexpectet type");
	return lua::stack_pop<R>(L);
}

std::string read_value_attribute(lua_State * L)
{
	table_field(L, "value");
	assert(lua_istable(L, -1) && "table expected");

	std::string name = refers_to_gloabal_variable(L);
	lua_pop(L, 1);  // pops value

	return name;
}

lua_node_resource::lua_node_resource(resource_descriptor * desc, resource_manager * resman)
{
	auto ldesc = dynamic_cast<lua_resource_descriptor *>(desc);
	lua_State * L = ldesc->L;

	lua::table tb(L);  // {type='node', ...}
	assert(tb.at("type") == "node" && "node element expected");

	for (lua::table_pair kv : tb)
	{
		if (kv.key_type() == LUA_TSTRING)  // attributes
		{
			if (kv.key() == "flags")
				append_flag(kv.value());
		}
		else if (kv.key_type() == LUA_TNUMBER)  // elements
		{
			lua::table tb(L);
			std::string type = tb.at("type");
			if (type == "node")
			{
				ptr<node> child;
				std::string value;
				if (tb.at("value", value))
					child = resman->load_resource<node>(value);
				else
					child = make_ptr<lua_node_resource>(desc, resman);

				if (child)
					append_child(child);
			}  // node
			else if (type == "method")
			{
				ptr<task_factory> factory = resman->load_resource<task_factory>(read_value_attribute(L));
				if (factory)
				{
					ptr<method> m = make_ptr<method>(factory);

					bool enabled;
					if (tb.at("enabled", enabled))
						m->enable(enabled);

					assoc_method(tb.at("id"), m);
				}
			}  // method
		}
		else
			assert(false && "unsupported element type");
	}  // for
}

lua_foreach_factory_resource::lua_foreach_factory_resource(resource_descriptor * desc, resource_manager * resman)
{
	auto ldesc = dynamic_cast<lua_resource_descriptor *>(desc);
	lua_State * L = ldesc->L;

	lua::table tb(L);  // {type='foreach', var=<string>, flag=<string>, [culling=<bool>, parallel=<bool>]}
	assert(tb.at("type") == "foreach" && "foreach element expected");

	std::string var;
	std::string flag;
	bool cull = true;
	bool parallel = true;

	tb.at("var", var);
	tb.at("flag", flag);
	tb.at("culling", cull);
	tb.at("parallel", parallel);

	ptr<task_factory> subtask;
	for (auto kv : tb)
	{
		if (kv.key_type() == LUA_TNUMBER)  // elements
		{
			lua::table tb(L);
			std::string type = tb.at("type");
			if (type == "callmethod")
			{
				lua_resource_descriptor desc(type, L);
				subtask = std::dynamic_pointer_cast<task_factory>(resource_factory::ref().create(&desc, resman));
			}
		}
		else if (kv.key_type() != LUA_TSTRING)
			assert(false && "unsupported table field key type (use number or string)");
	}

	assert(subtask && "empty foreach loop in lua script");  // TODO: let lua check this

	init(var, flag, cull, parallel, subtask);
}

lua_callmethod_factory_resource::lua_callmethod_factory_resource(resource_descriptor * desc, resource_manager * resman)
{
	auto ldesc = dynamic_cast<lua_resource_descriptor *>(desc);
	lua_State * L = ldesc->L;

	lua::table tb(L);  // {type='callmethod', <qualified-name>}
	assert(tb.at("type") == "callmethod" && "callmethod element expected");

	std::string meth_name = tb.at<std::string>(1);

	init(meth_name);
}
