/*! \file luatools.hpp
\author Adam Hlavatovič */
#pragma once
#include <map>
#include <vector>
#include <string>
#include <utility>
#include <cassert>
#include <lua.hpp>
#include "script_engine.hpp"
#include "stack.hpp"
#include "array.hpp"
#include "map.hpp"

namespace lua {

inline lua_State * newstate() {return luaL_newstate();}

template <typename Value, typename Key>
Value table_value(lua_State * L, Key k, int sidx)  // TODO: table field
{
	stack_push(L, k);
	lua_gettable(L, sidx-1);
	return stack_pop<Value>(L);
}

/*! \note Argumenty sú v prúde v opačnom poradí ako ich vracia volaná (lua)
funkcia. */
class istack_stream
{
public:
	using self = istack_stream;

	istack_stream(result const & r) : istack_stream(r.state()) {}
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
};  // istack_stream

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

