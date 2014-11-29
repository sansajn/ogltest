#include "lua_tweakbar.hpp"
#include "luatools/table.hpp"
#include "resource/resource_manager.hpp"

static std::vector<twbar_variable_desc> read_tweakbar_as_lua(lua_State * L);
static twbar_variable_desc bool_element(lua_State * L);
static twbar_variable_desc float_element(lua_State * L);

lua_draw_tweakbar_resource::lua_draw_tweakbar_resource(resource_descriptor * desc, resource_manager * resman)
{
	lua_State * L = state(desc);
	lua::table tb(L);
	assert(tb.at("type") == "drawtweakbar" && "drawtweakbar element expected");
	// do nothing, just create ...
}

lua_tweakbar_manager_resource::lua_tweakbar_manager_resource(resource_descriptor * desc, resource_manager * resman)
{
	lua_State * L = state(desc);
	lua::table tb(L);  // tweakbar_manager{type='tweakbar_manager', editor{}. ...}
	assert(tb.at("type") == "tweakbar_manager" && "tweakbar_manager element expected");

	std::vector<bar_data> bars;
	for (lua::field_pair fp : tb|lua::only_number_keys)  // elements
	{
		lua::table tb(L);
		std::string type = tb.at("type");
		if (type == "editor")
		{
			std::string id = tb.at("id");
//			TODO: odstran atribut bar, aby sa dal bar definovat ako literal (nie premenna), takto
//				editor{
//					id='resources', permanent=true, exclusive=false,
//					tweakbar{
//						float{label='Exposure', keyincr='+', keydecr='-', group='Rendering',
//							min=0.01, max=10.0, step=0.01
//					}
//				}
			ptr<tweakbar_handler> handler = resman->load_resource<tweakbar_handler>(get_variable_name(L, "bar"));
			bool exclusive = tb.field<bool>("exclusive", false);
			bool permanent = tb.field<bool>("permanent", false);
			char key = tb.field<char>("key", '\0');
			bar_data bar(handler, exclusive, permanent, key);
			bars.push_back(bar);  // TODO: implace
		}
		else
			assert(false && "unknown element (only editor element supported)");
	}

	init(bars);
}

lua_tweakbar_resource::lua_tweakbar_resource(resource_descriptor * desc, resource_manager * resman)
{
	lua_State * L = state(desc);
	std::vector<twbar_variable_desc> vars = read_tweakbar_as_lua(L);
	init(vars, resman);
}

std::vector<twbar_variable_desc> read_tweakbar_as_lua(lua_State * L)
{
	lua::table tb(L);
	assert(tb.at("type") == "tweakbar" && "'tweakbar' element expected");

	std::vector<twbar_variable_desc> vars;

	string label = tb.at("label");
	string name = tb.field("name");

	for (auto fp : tb|lua::only_number_keys)
	{
		lua::table tb(L);
		string type = tb.at("type");
		if (type == "float")
			vars.push_back(float_element(L));
		else if (type == "bool")
			vars.push_back(bool_element(L));
		else
			assert(false && "only floats and bools supported (implement)");
	}

	return vars;
}

twbar_variable_desc bool_element(lua_State * L)
{
	twbar_variable_desc var;
	var.type = TW_TYPE_BOOLCPP;

	lua::table tb(L);
	var.label = tb.at("label");
	var.path = tb.at("path");
	var.group = tb.field("group");
	var.help = tb.field("help");

	return var;
}

twbar_variable_desc float_element(lua_State * L)
{
	twbar_variable_desc var;
	var.type = TW_TYPE_FLOAT;

	lua::table tb(L);
	var.label = tb.at("label");
	var.path = tb.at("path");
	var.group = tb.field("group");
	var.help = tb.field("help");
	var.keyincr = tb.field("keyincr");
	var.keydecr = tb.field("keydecr");
	tb.at<float>("min", var.min);
	tb.at<float>("max", var.max);
	tb.at<float>("step", var.step);

	return var;
}
