/*! \file luatools.hpp
\author Adam Hlavatovič */
#pragma once
#include <map>
#include <vector>
#include <string>
#include <cassert>

#ifdef WIN32
	#include <lua.hpp>
	#pragma warning(disable:4800)
#else
	#include <lua.hpp>
#endif


namespace loe {
	namespace lua {

typedef void (* errout)(char const * msg);

inline lua_State * newstate() {return luaL_newstate();}

//! Lua Virtual Machine.
class vm
{
public:
	vm(errout luaerr) : _luaerr(luaerr) {}
	void init(lua_State * L);
	void register_function(lua_State * L, lua_CFunction, char const * lname);

	//! \note Stack: argn, ..., arg1, <chunk-name>
	int call_function(lua_State * L, char const * lname, int narg);
	int run_script(lua_State * L, char const * fname);

private:
	int report(lua_State * L, int state);
	int call_chunk(lua_State * L, int narg);

	errout _luaerr;
};


//! Lua stack low-level manipulators.
//@{
template <typename R>
inline R stack_at(lua_State * L, int idx);

template <>
inline int stack_at<int>(lua_State * L, int idx)
{
	return lua_tointeger(L, idx);
}

template <>
inline double stack_at<double>(lua_State * L, int idx)
{
	return lua_tonumber(L, idx);
}

template <>
inline std::string stack_at<std::string>(lua_State * L, int idx)
{
	return lua_tostring(L, idx);
}

template <>
inline bool stack_at<bool>(lua_State * L, int idx)
{
	return lua_toboolean(L, idx) == 1;
}


template <typename T>
void stack_push(lua_State * L, T const & x);

template <> 
inline void stack_push<int>(lua_State * L, int const & x)
{
	lua_pushinteger(L, x);
}

template <> 
inline void stack_push<double>(lua_State * L, double const & x)
{
	lua_pushnumber(L, x);
}

template <> 
inline void stack_push<std::string>(lua_State * L, std::string const & x)
{
	lua_pushstring(L, x.c_str());
}

//! \note: nie je šablonova špecializacia, kvoly pointru x
inline void stack_push(lua_State * L, char const * x)
{
	lua_pushstring(L, x);
}

template <>
inline void stack_push<bool>(lua_State * L, bool const & x)
{
	lua_pushboolean(L, x ? 1 : 0);
}


template <typename R>
inline R stack_pop(lua_State * L);

template <> 
inline int stack_pop<int>(lua_State * L)
{
	int tmp = lua_tointeger(L, -1);
	lua_pop(L, 1);
	return tmp;
}

template <> 
inline double stack_pop<double>(lua_State * L)
{
	double tmp = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return tmp;
}

template <> 
inline std::string stack_pop<std::string>(lua_State * L)
{
	std::string tmp = lua_tostring(L, -1);
	lua_pop(L, 1);
	return tmp;
}

template <>
inline bool stack_pop<bool>(lua_State * L)
{
	bool tmp = lua_toboolean(L, -1) == 1;
	lua_pop(L, 1);
	return tmp;
}


template <typename Value, typename Key>
Value table_value(lua_State * L, Key k, int sidx) 
{
	stack_push(L, k);
	lua_gettable(L, sidx-1);
	return stack_pop<Value>(L);
}
//@}

template <typename T>
class array_range
{
public:
	array_range(lua_State * L, int sidx) : _L(L), _sidx(sidx), _tidx(1) {}
	void operator++() {++_tidx;}

	T operator*() {
		lua_rawgeti(_L, _sidx, _tidx);
		return stack_pop<T>(_L);
	}

	operator bool() {return _tidx <= lua_rawlen(_L, _sidx);}

private:
	lua_State * _L;
	int _tidx;  //!< table index
	int const _sidx;  //!< stack index
};  // array_range

// iteracia skrz homogennu tabulku
template <typename T>
class map_range
{
public:
	map_range(lua_State * L, int sidx = -1) : _L(L) {
		_sidx = lua_gettop(_L) + (sidx + 1);
		lua_pushnil(_L);
		_ok = lua_next(_L, _sidx);
	}

	void operator++() {
		lua_pop(_L, 1);
		_ok = lua_next(_L, _sidx);
	}

	std::pair<std::string, T> operator*() {
		return std::make_pair(stack_at<std::string>(_L, -2), stack_at<T>(_L, -1));
	}

	operator bool() const {return _ok;}

private:
	bool _ok;
	int _sidx;  //!< table stack index
	lua_State * _L;
};  // map_range


/*! Iteracia skrz heterogennu tabulku.
\code
	for (table_range r(L); r; ++r) {
		if (r.key() == "name")
			cout << "name:" << r.value<string>();
		else if (r.key() == "age")
			cout << "age:" << r.value<int>();
	}
\endcode */
class table_range
{
public:
	table_range(lua_State * L, int sidx = -1) : _L(L) {
		_sidx = lua_gettop(_L) + sidx + 1;
		lua_pushnil(_L);
		_ok = lua_next(_L, _sidx);
	}

	void operator++() {
		lua_pop(_L, 1);
		_ok = lua_next(_L, _sidx);
	}

	std::string key() {return stack_at<std::string>(_L, -2);}

	template <typename R>
	R value() {return stack_at<R>(_L, -1);}

	operator bool() const {return _ok;}

private:
	int _sidx;
	lua_State * _L;
	bool _ok;
};  // table_range


/*! \note Argumenty sú v prúde v opačnom poradí ako ich vracia volaná (lua)
funkcia. */
class istack_stream
{
public:
	typedef istack_stream self;

	istack_stream(lua_State * L, int sidx = -1) : _L(L), _sidx(sidx) {}

	//! unary-manipulators
	self & operator>>(void (*fn)(self & is)) {
		fn(*this);
		return *this;
	}

	template <typename T>
	self & operator>>(T & x)
	{
		x = stack_at<T>(_L, _sidx--);
		return *this;
	}

	// \todo pouzi genericku funkciu push_back
	self & operator>>(bool & val) {
		val = lua_toboolean(_L, _sidx--) == 1;
		return *this;
	}

	template <typename T>
	self & operator>>(std::vector<T> & val) {
		for (array_range<T> r = get_array<T>(); r; ++r)
			val.push_back(*r);
		return *this;
	}

	template <typename T>
	self & operator>>(std::map<std::string, T> & val) {
		for (map_range<T> r = get_map<T>(); r; ++r)
		{
			std::pair<std::string, T> p = *r;
			val[p.first] = p.second;
		}
		return *this;
	}

	void next() {--_sidx;}

	//! \todo premenuj
	template <typename Value, typename Key>
	Value table_value(Key k) {
		return lua::table_value<Value>(_L, k, _sidx);
	}

	lua_State * get() const {return _L;}

private:
	template <typename T>
	array_range<T> get_array() {
		return array_range<T>(_L, _sidx--);
	}

	template <typename T>
	map_range<T> get_map() {
		return map_range<T>(_L, _sidx--);
	}

	lua_State * _L;
	int _sidx;	//!< stack index
};

namespace detail {

// prmenovat, tento nazov mi nejako nepasuje, preco query v pripade outu ?
template <typename Key, typename Value>
struct table_query
{
	Key const key;
	Value & value;
	table_query(Key const & k, Value & v) : key(k), value(v) {}
};

struct binary_object
{
	char const * value;
	int size;
	binary_object(char const * buf, int n) : value(buf), size(n) {}
};

};  // detail

/*! \note Tabuľkový manipulátor implicitne neposunie ukazateľ na ďalší 
prvok v zásobníku. Ak chceme zo zásobníku čítať dalšie prvky, je potrebné 
zavolať manipulátor next(). */
template <typename Key, typename Value>
inline detail::table_query<Key, Value> tab(Key k, Value & v)
{
	return detail::table_query<Key, Value>(k, v);
}


template <typename Key, typename Value>
inline istack_stream & operator>>(istack_stream & is, 
	detail::table_query<Key, Value> & f)
{
	f.value = is.table_value<Value>(f.key);
	return is;
}

/* \note Pri globálnych operátorou<< je výraz [Ukážka:

	ostack_stream(L) << variable;

--- koniec ukážky] neplatný. Na ľavej strane je r-value objekt, ktorého
'zachitenie' si vyžaduje operátor pre konštantný ostack_stream [Ukážka:

	reference operator<<(ostack_stream const &, T const &);

--- koniec ukážky]. V implementácii operátora je potom možné volať iba
konštantné metódy, čo nemusí byť vždy možné. */
class ostack_stream
{
public:
	typedef ostack_stream self;

	ostack_stream(lua_State * L) : _L(L) {}

	
	template <typename T>
	self & operator<<(T const & rhs) {
		stack_push(_L, rhs);
		return *this;
	}

	//! unary-manipulators
	self & operator<<(void (*fn)(self & os)) {
		fn(*this);
		return *this;
	}

	template <typename T>
	self & operator<<(std::vector<T> const & val) {
		lua_newtable(_L);
		for (int i = 0; i < val.size(); ++i) {
			*this << val[i];
			lua_rawseti(_L, -2, i+1);
		}
		return *this;
	}

	/* \note, externý operátor nemôžem použit s dočastným objektom takto

		ostack_stream(L) << tab("one", i)

	Zda sa že interny môžem. */
	template <typename Key, typename Value>
	self & operator<<(detail::table_query<Key, Value> const & q) {
		assert(lua_istable(_L, -1) && "table expected");
		*this << q.value;
		lua_setfield(_L, -2, q.key);
		return *this;
	}

	// podpora pre binarne data
	self & operator<<(detail::binary_object const & rhs) {
		lua_pushlstring(_L, rhs.value, rhs.size);
		return *this;
	}

	lua_State * get() const {return _L;}

private:
	lua_State * _L;
};


//! Stack-stream manipulators
//@{

//! Posunie sa na ďalší prvok v zásobníku.
inline void next(istack_stream & is) {is.next();}

inline void newtable(ostack_stream & os) {lua_newtable(os.get());}

inline detail::binary_object binary(char const * src, int size) 
{
	return detail::binary_object(src, size);
}

//@}

	};  // lua
};  // loe

