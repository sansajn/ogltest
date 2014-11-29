#pragma once
#include <boost/range/adaptors.hpp>
#include "script_engine.hpp"
#include "stack.hpp"

namespace lua {

class field_pair
{
public:
	field_pair(lua_State * L) : _L(L) {}

	template <typename T>
	bool key_is() const {return istype<T>();}

	template <typename T>
	bool value_is() const {return istype<T>();}

	template <typename R = std::string>
	R key() const {return stack_at<R>(_L, -2);}

	template <typename R = std::string>
	R value() const {return stack_at<R>(_L, -1);}

	field_pair * operator->() {return this;}

	/*! \return returns lua type
	\saa LUA_TNIL, LUA_TNUMBER, LUA_TBOOLEAN, LUA_TSTRING, LUA_TTABLE, LUA_TFUNCTION, LUA_TUSERDATA, LUA_TTHREAD, and LUA_TLIGHTUSERDATA */
	int key_type() const {return lua_type(_L, -2);}
	int value_type() const {return lua_type(_L, -1);}

private:
	lua_State * _L;
};

class table_iterator
	: public std::iterator<std::forward_iterator_tag, field_pair, std::ptrdiff_t, field_pair *, field_pair>
{
public:
	table_iterator(lua_State * L);

	void operator++(int) {++*this;}
	table_iterator & operator++();
	field_pair operator*() const {return field_pair(_L);}
	bool operator==(table_iterator const & rhs) const {return _L == rhs._L;}
	bool operator!=(table_iterator const & rhs) const {return _L != rhs._L;}

private:
	void next();

	lua_State * _L;
};

/*! Iteracia skrz heterogennu tabulku.
\code
	result res = lvm.call_function("some_lua_function");
	for (table_pair tp : table_range(res)) {
		if (tp.key() == "name")
			cout << "name:" << tp.value<string>();
		else if (tp.key() == "age")
			cout << "age:" << tp.value<int>();
	}
\endcode */
class table_range
{
public:
	table_range(table_iterator beg, table_iterator end) : _it(beg), _end(end) {}
	table_range(result const & res) : table_range(table_iterator(res.state()), table_iterator(nullptr)) {}
	table_range(lua_State * L) : table_range(table_iterator(L), table_iterator(nullptr)) {}

	void operator++() {++_it;}
	field_pair operator*() const {return *_it;}
	field_pair operator->() const {return *_it;}
	operator bool() const {return _it != _end;}
	table_iterator begin() const {return _it;}
	table_iterator end() const {return _end;}

private:
	table_iterator _it, _end;
};

template <typename K>
void table_field(lua_State * L, K const & key)
{
	assert(lua_istable(L, -1) && "table expected");
	stack_push(L, key);
	lua_gettable(L, -2);
}

namespace detail {

template <typename V, typename K>
class const_field_ref
{
public:
	const_field_ref(lua_State * L, K const & key) : _L(L) {table_field(_L, key);}
	operator V() const {return stack_pop<V>(_L);}

private:
	lua_State * _L;
};

template <typename V, typename K>
class field_ref
{
public:
	field_ref(lua_State * L, K const & key) : _L(L), _sidx(lua_gettop(_L)) {stack_push(L, key);}

	~field_ref()
	{
		if (lua_gettop(_L) > _sidx)
			lua_pop(_L, _sidx - lua_gettop(_L));
	}

	template <typename T>
	void operator=(T const & rhs)
	{
		stack_push(_L, rhs);
		lua_settable(_L, -3);  // value, key, table, ..., bottom
	}

	operator V() const
	{
		lua_gettable(_L, -2);
		return stack_pop<V>(_L);
	}

private:
	lua_State * _L;
	int _sidx;
};


}  // lua_detail

/*! Umožnuje prístup do tabuľky na zásobníku.
\code
	table tb(L);
	tb.at<int>("name") = 123;
	int name = tb.at<int>("name");  // name=123
	for (auto kv : tb)
		cout << kv.key() << ":" << kv.value() << "\n";
	table subtb = tb.at<table>("subtable");
	...
\endcode */
class table
{
public:
	typedef table_iterator iterator;
	typedef table_iterator const_iterator;

	table(lua_State * L, bool destroy = false) : _L(L), _sidx(lua_gettop(L)), _destroy(destroy) {}
	table(result const & r) : table(r.state()) {}
	table(table && rhs);
	~table();

	void newtable();
	void pop();  //!< pops table trom stack

	template <typename V = std::string, typename K = std::string>  //! \note pole musi byt v tabulke
	detail::const_field_ref<V, K> at(K const & key) const
	{
		return detail::const_field_ref<V, K>(_L, key);
	}

	template <typename V = std::string, typename K = std::string>
	detail::field_ref<V, K> at(K const & key) {return detail::field_ref<V, K>(_L, key);}

	template <typename V, typename K>
	bool at(K const & key, V & value) const  // TODO: nech to pracuje aj s tabulkou
	{
		table_field(_L, key);
		bool contains = !lua_isnil(_L, -1);
		if (contains)
		{
			value = stack_at<V>(_L, -1);
			assert(istype<V>(_L) && "unexpected field type");
		}
		lua_pop(_L, 1);  // pops value
		return contains;
	}

	template <typename V = std::string, typename K = std::string>
	V field(K const & key, V const & default_value = V()) const
	{
		V value;
		if (at(key, value))
			return value;
		else
			return default_value;
	}

	template <typename K>
	bool contains(K const & key) const
	{
		stack_push(_L, key);
		lua_gettable(_L, -2);
		bool res = !lua_isnil(_L, -1);
		lua_pop(_L, 1);
		return res;
	}

	table_iterator begin() const {return table_iterator(_L);}
	table_iterator end() const {return table_iterator(nullptr);}

	table(table const &) = delete;
	table & operator=(table const &) = delete;

private:
	lua_State * _L;
	int _sidx;
	bool _destroy;  //!< table is new
};  // table


namespace detail {

template <typename K>  // specializacia field_ref<V, K>
struct const_field_ref<table, K>
{
	const_field_ref(lua_State * L, K const & key) : _L(L) {table_field(_L, key);}
	operator table() {return table(_L, true);}
	lua_State * _L;
};

template <typename K>  // specializacia ref<V, K>
struct field_ref<table, K>
{
	field_ref(lua_State * L, K const & key) : _L(L) {table_field(_L, key);}
	operator table() {return table(_L, true);}
	lua_State * _L;
};

}  // lua_detail


/*! Boost::Range filter helpers
\code
for (auto v : table(L)|only_number_keys)
	...
\endcode
*/
//@{

namespace detail {

struct number_keys_pred
{
	bool operator()(lua::field_pair const & v) const {return v.key_type() == LUA_TNUMBER;}
};

struct string_keys_pred
{
	bool operator()(lua::field_pair const & v) const {return v.key_type() == LUA_TSTRING;}
};

}  // detail

namespace {

boost::range_detail::filter_holder<detail::number_keys_pred> only_number_keys(detail::number_keys_pred{});
boost::range_detail::filter_holder<detail::string_keys_pred> only_string_keys(detail::string_keys_pred{});

}

//@}

}  // lua

template <typename V, typename K>
bool operator==(lua::detail::field_ref<V, K> const & lhs, V const & rhs)
{
	return V(lhs) == rhs;
}

template <typename K>
bool operator==(lua::detail::field_ref<std::string, K> const & lhs, std::string const & rhs)
{
	return std::string(lhs) == rhs;
}
