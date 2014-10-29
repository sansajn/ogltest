#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <utility>
#include <cstring>
#include <cassert>

#define BOOST_TEST_MODULE lua_tools_test
#include <boost/test/included/unit_test.hpp>

#include "luatools.hpp"

using std::string;
using std::vector;
using std::map;
using std::copy;

void lmessage(char const * msg);

// testuje call_function
BOOST_AUTO_TEST_CASE(test_callfunc)
{
	lua::vm lvm(lmessage);
	lvm.load_script("test.lua");

	lvm.call_function("sayhello");

	BOOST_CHECK(lua_gettop(lvm.state()) == 0);

	{
		lua::result r = lvm.call_function("echo", 11);
		int n = 0;
		lua::istack_stream(r) >> n;
		BOOST_CHECK(n == 11);
	}

	BOOST_CHECK(lua_gettop(lvm.state()) == 0);
}

BOOST_AUTO_TEST_CASE(test_table_range)
{
	lua::vm lvm(lmessage);
	lvm.load_script("test.lua");

	{
		lua::result r = lvm.call_function("return_table");
		for (lua::table_range rgn(r); rgn; ++rgn)
		{
			if (rgn.key() == "name")
				BOOST_CHECK(rgn.value<string>() == "Peter");
			else if (rgn.key() == "age")
				BOOST_CHECK(rgn.value<int>() == 33);
		}
	}

	BOOST_CHECK(lua_gettop(lvm.state()) == 0);
}

BOOST_AUTO_TEST_CASE(test_boolean)
{
	lua::vm lvm(lmessage);
	lua_State * L = lvm.state();
	lvm.load_script("test.lua");

	lua::ostack_stream(L) << true;
	lvm.call_function_raw("echo_boolean", 1);
	bool received;
	lua::istack_stream(L) >> received;
	BOOST_CHECK(received == true);
	lua_pop(L, 1);

	lua::ostack_stream(L) << false;
	lvm.call_function_raw("echo_boolean", 1);
	lua::istack_stream(L) >> received;
	BOOST_CHECK(received == false);
	lua_pop(L, 1);

	BOOST_CHECK(lua_gettop(L) == 0);
}

BOOST_AUTO_TEST_CASE(test_error)
{
	lua::vm lvm(lmessage);
	lua_State * L = lvm.state();
	lvm.load_script("test.lua");

	lvm.call_function_raw("simple_error", 0);

	BOOST_CHECK(lua_gettop(L) == 0);
}

BOOST_AUTO_TEST_CASE(test_ostream_table)
{
	lua::vm lvm(lmessage);
	lua_State * L = lvm.state();
	lvm.load_script("test.lua");

	int i = 1, j = 2, k = 3;

/*
	lua::ostack_stream los(L);
	los << lua::newtable << lua::tab("one", i)
		<< lua::tab("two", j) << lua::tab("tree", k);
*/

	lua::ostack_stream(L) << lua::newtable << lua::tab("one", i)
		<< lua::tab("two", j) << lua::tab("tree", k);

	lvm.call_function_raw("echo_table", 1);

	map<string, int> received;
	lua::istack_stream(L) >> received;

	BOOST_CHECK(received.size() == 3);
	BOOST_CHECK(received["one"] == 1);
	BOOST_CHECK(received["two"] == 2);
	BOOST_CHECK(received["tree"] == 3);

	lua_pop(L, 1);

	BOOST_CHECK(lua_gettop(L) == 0);
}

BOOST_AUTO_TEST_CASE(test_io)
{
	lua::vm lvm(lmessage);
	lua_State * L = lvm.state();
	lvm.load_script("test.lua");

	lvm.call_function_raw("write_test", 0);

	BOOST_CHECK(lua_gettop(L) == 0);
}

BOOST_AUTO_TEST_CASE(test_luasql)
{
	lua::vm lvm(lmessage);
	lua_State * L = lvm.state();
	lvm.load_script("test.lua");

	lvm.call_function_raw("sqlite3_test", 0);

	BOOST_CHECK(lua_gettop(L) == 0);
}

BOOST_AUTO_TEST_CASE(test_arrayos)
{
	lua::vm lvm(lmessage);
	lua_State * L = lvm.state();
	lvm.load_script("test.lua");

	vector<double> x{0.12, 2.34, 6.98, 0.02};
	lua::ostack_stream(L) << x;
	lvm.call_function_raw("filter", 1);

	vector<double> y;
	lua::istack_stream(L) >> y;

	vector<double> z;
	for (auto & e : x)
		z.push_back(e*e);

	BOOST_CHECK(z == y);

	lua_pop(L, 1);

	BOOST_CHECK(lua_gettop(L) == 0);
}

BOOST_AUTO_TEST_CASE(test_arrayis)
{
	lua::vm lvm(lmessage);
	lua_State * L = lvm.state();
	lvm.load_script("test.lua");

	lvm.call_function_raw("dummy_table", 0);
	vector<int> v;
	lua::istack_stream(L) >> v;

	vector<int> expected_data{1, 2, 3, 2, 1};

	BOOST_CHECK(v == expected_data);

	lua_pop(L, 1);

	BOOST_CHECK(lua_gettop(L) == 0);
}

BOOST_AUTO_TEST_CASE(test_arrayr)
{
	lua::vm lvm(lmessage);
	lua_State * L = lvm.state();
	lvm.load_script("test.lua");

	lvm.call_function_raw("dummy_table", 0);

	int expected_data[] = {1, 2, 3, 2, 1};

	lua::array_range<int> r(L, -1);

	for (int i = 0; r; ++r, ++i)
		BOOST_CHECK(*r == expected_data[i]);

	lua_pop(L, 1);

	BOOST_CHECK(lua_gettop(L) == 0);
}

BOOST_AUTO_TEST_CASE(test_test)
{
	lua::vm lvm(lmessage);
	lua_State * L = lvm.state();
	lvm.load_script("test.lua");

	lvm.call_function_raw("test", 0);

	string name;
	int age;
	double weigth;

	lua::istack_stream(L) >> weigth >> age >> name;

	BOOST_CHECK(name == "Frenk");
	BOOST_CHECK(age == 22);
	BOOST_CHECK(weigth == 87.5);

	lua_pop(L, 3);

	BOOST_CHECK(lua_gettop(L) == 0);
}

BOOST_AUTO_TEST_CASE(test_person)
{
	lua::vm lvm(lmessage);
	lua_State * L = lvm.state();
	lvm.load_script("test.lua");

	lua::ostack_stream(L) << "Frenk Black";
	lvm.call_function_raw("person_data", 1);

	int salary, age;
	lua::istack_stream(L) >> salary >> age;

	BOOST_CHECK(age == 25);
	BOOST_CHECK(salary == 1890);

	lua_pop(L, 2);

	BOOST_CHECK(lua_gettop(L) == 0);
}

BOOST_AUTO_TEST_CASE(test_return_table)
{
	lua::vm lvm(lmessage);
	lua_State * L = lvm.state();
	lvm.load_script("test.lua");

	lvm.call_function_raw("return_table", 0);

	for (lua::table_range r(L); r; ++r)
	{
		if (r.key() == "name")
			BOOST_CHECK(r.value<string>() == "Peter");
		else if (r.key() == "age")
			BOOST_CHECK(r.value<int>() == 33);
	}

	lua_pop(L, 1);

	BOOST_CHECK(lua_gettop(L) == 0);
}

BOOST_AUTO_TEST_CASE(test_return_mixed_table)
{
	// {1, 2, name='Lisbon'}
	lua::vm lvm(lmessage);
	lvm.load_script("test.lua");

	lua::result res = lvm.call_function("return_mixed_table");

	int idx = 0;
	for (lua::table_range r(res); r; ++r, ++idx)
	{
		std::pair<int, int> types = *r;
		if (types.first == LUA_TSTRING)
		{
			if (r.key() == "name")
				BOOST_CHECK_EQUAL(r.value<string>(), string("Lisbon"));
		}
		else
			BOOST_CHECK_EQUAL(idx+1, r.value<int>());
	}
}

// TODO: one-line call test lua::table_range(lvm.call_function()))

struct person
{
	string name;
	int age;
};

BOOST_AUTO_TEST_CASE(test_custom_structure)
{
	lua::vm lvm(lmessage);
	lua_State * L = lvm.state();
	lvm.load_script("test.lua");

	person p = {"Franta Spelec", 29};

	lua::ostack_stream(L) << lua::newtable << lua::tab("name", p.name)
		<< lua::tab("age", p.age);

	lvm.call_function_raw("custom_structure_test", 1);

	int result = 0;
	lua::istack_stream(L) >> result;

	BOOST_CHECK(result == 1);

	lua_pop(L, 1);

	BOOST_CHECK(lua_gettop(L) == 0);
}

namespace lua {

template <>
inline void stack_push<person>(lua_State * L, person const & p)
{
	ostack_stream(L) << newtable << tab("name", p.name) << tab("age", p.age);
}

}  // lua

BOOST_AUTO_TEST_CASE(custom_structure_overload)
{
	lua::vm lvm(lmessage);
	lua_State * L = lvm.state();
	lvm.load_script("test.lua");

	person p = {"Franta Spelec", 29};

	lua::ostack_stream(L) << p;

	lvm.call_function_raw("custom_structure_test", 1);

	int result = 0;
	lua::istack_stream(L) >> result;

	BOOST_CHECK(result == 1);

	lua_pop(L, 1);

	BOOST_CHECK(lua_gettop(L) == 0);
}

BOOST_AUTO_TEST_CASE(custom_structure_upload)
{
	lua::vm lvm(lmessage);
	lvm.load_script("test.lua");

	{
		person p = {"Franta Spelec", 29};
		lua::result r = lvm.call_function("custom_structure_test", p);

		int call_result = 0;
		lua::istack_stream(r) >> call_result;

		BOOST_CHECK(call_result == 1);
	}

	BOOST_CHECK(lua_gettop(lvm.state()) == 0);
}

BOOST_AUTO_TEST_CASE(custom_structure_array)
{
	lua::vm lvm(lmessage);
	lua_State * L = lvm.state();
	lvm.load_script("test.lua");

	person p[] = {
		{"Franta Spelec", 29},
		{"John Rambo", 33},
		{"Janosikov sveter", 26}
	};

	vector<person> arr(p, p+3);

	lua::ostack_stream(L) << arr;
	lvm.call_function_raw("custom_structure_array_test", 1);

	int result = 0;
	lua::istack_stream(L) >> result;

	BOOST_CHECK(result == 1);

	lua_pop(L, 1);

	BOOST_CHECK(lua_gettop(L) == 0);
}

BOOST_AUTO_TEST_CASE(test_ostream_binary)
{
//	lua::vm lvm(lmessage);
//	lua_State * L = lua::newstate();
//	lvm.init(L);
//	lvm.run_script(L, "test.lua");

//	char buf[15];
//	for (int i = 0; i < 15; ++i)
//		buf[i] = i+1;

//	int expected_result = 15*(15+1)/2;

//	lua::ostack_stream(L) << lua::binary(buf, 15);
//	lvm.call_function(L, "ostream_binary_test", 1);

//	int sum = 0;
//	lua::istack_stream(L) >> sum;

//	BOOST_CHECK(sum == expected_result);

//	lua_pop(L, 1);

//	BOOST_CHECK(lua_gettop(L) == 0);

//	lua_close(L);
}

BOOST_AUTO_TEST_CASE(test_ostream_binary_table)
{
//	lua::vm lvm(lmessage);
//	lua_State * L = lua::newstate();
//	lvm.init(L);
//	lvm.run_script(L, "test.lua");

//	char buf[15];
//	for (int i = 0; i < 15; ++i)
//		buf[i] = i+1;

//	int expected_result = 15*(15+1)/2;

//	auto bindata = lua::binary(buf, 15);

//	auto lt = lua::tab("raw", bindata);
//	lua::ostack_stream(L) << lt;
//	lvm.call_function(L, "ostream_binary_test_table", 1);

//	int sum = 0;
//	lua::istack_stream(L) >> sum;

//	BOOST_CHECK(sum == expected_result);

//	lua_pop(L, 1);

//	BOOST_CHECK(lua_gettop(L) == 0);

//	lua_close(L);
}


void lmessage(char const * msg)
{
	fprintf(stderr, "%s\n", msg);
	fflush(stderr);
}
