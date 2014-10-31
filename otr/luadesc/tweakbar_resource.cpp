#include "tweakbar_resource.hpp"

#define CHECK_TABLE() (assert(lua_istable(L, -1) && "table expected"))
#define CHECK_ATTRIBUTE() (assert(r.key_type() == LUA_TSTRING && "only attributes allowed"))


twbar_variable_desc bool_element(lua_State * L);
twbar_variable_desc float_element(lua_State * L);
string build_definition_string(twbar_variable_desc const & var);


void TW_CALL SetCallback(const void * value, void * client_data)
{
//	myVariable = *(const MyVariableType *)value;  // for instance
}

void TW_CALL GetCallback(void * value, void * client_data)
{
//	*(MyVariableType *)value = myVariable;  // for instance
}

tweakbar_resource::tweakbar_resource(std::vector<twbar_variable_desc> const & vars)
	: tweakbar_handler("dummy", true), _vars(vars)
{
	_fvar = 10.10;
	_bvar = true;
}

void tweakbar_resource::update_bar(TwBar * bar)
{
	for (twbar_variable_desc const & var : _vars)
	{
		string def = build_definition_string(var);

		switch (var.type)
		{
			case TW_TYPE_BOOLCPP:
				TwAddVarRW(bar, var.label.c_str(), TW_TYPE_BOOLCPP, &_bvar, def.c_str());
				break;

			case TW_TYPE_FLOAT:
				TwAddVarRW(bar, var.label.c_str(), TW_TYPE_FLOAT,&_fvar, def.c_str());
				break;

			default:
				assert(false && "type is not supported");  // TODO: suport others
				break;
		}
	}  // for
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

std::vector<twbar_variable_desc> read_tweakbar_as_lua(std::string const & script)
{
	lua::vm lvm(lua::lmessage);
	lvm.load_script(script.c_str());
	lua_State * L = lvm.state();

	lua_getglobal(L, "content");
	CHECK_TABLE();

	{
		table tbl(L);
		string type = tbl.at<string>("type");
		assert(type == "tweakbar" && "unexpected element type, 'tweakbar' expected");
	}

	std::vector<twbar_variable_desc> vars;

	string name, label;
	for (lua::table_range r(L); r; ++r)
	{
		if (r.key_type() == LUA_TSTRING)  // attributes
		{
			if (r.key() == "name")
				name = r.value<string>();
			else if (r.key() == "label")
				label = r.value<string>();
		}
		else if (r.key_type() == LUA_TNUMBER)  // elements
		{
			table subtbl(L);
			string type = subtbl.at<string>("type");
			if (type == "float")
				vars.push_back(float_element(L));
			else if (type == "bool")
				vars.push_back(bool_element(L));
		}
	}  // for

	return vars;
}

twbar_variable_desc bool_element(lua_State * L)
{
	twbar_variable_desc var;
	var.type = TW_TYPE_BOOLCPP;

	for (lua::table_range r(L); r; ++r)
	{
		CHECK_ATTRIBUTE();
		if (r.key() == "label")
			var.label = r.value<string>();
		else if (r.key() == "group")
			var.group = r.value<string>();
		else if (r.key() == "help")
			var.help = r.value<string>();
	}
	return var;
}

twbar_variable_desc float_element(lua_State * L)
{
	twbar_variable_desc var;
	var.type = TW_TYPE_FLOAT;

	for (lua::table_range r(L); r; ++r)
	{
		CHECK_ATTRIBUTE();
		if (r.key() == "label")
			var.label = r.value<string>();
		else if (r.key() == "group")
			var.group = r.value<string>();
		else if (r.key() == "help")
			var.help = r.value<string>();
		else if (r.key() == "keyincr")
			var.keyincr = r.value<string>();
		else if (r.key() == "keydecr")
			var.keydecr = r.value<string>();
		else if (r.key() == "min")
			var.min = r.value<float>();
		else if (r.key() == "max")
			var.max = r.value<float>();
		else if (r.key() == "step")
			var.step = r.value<float>();
	}
	return var;
}
