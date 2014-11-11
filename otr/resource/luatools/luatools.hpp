/*! \file luatools.hpp
\author Adam Hlavatovič */
#pragma once
#include <map>
#include <vector>
#include <string>
#include <utility>
#include <cassert>
#include <lua.hpp>

namespace lua {

typedef void (* errout)(char const * msg);

inline lua_State * newstate() {return luaL_newstate();}

//! Lua stack low-level manipulators.
//@{
template <typename R>
R stack_at(lua_State * L, int idx);

template <typename T>
void stack_push(lua_State * L, T const & x);

template <typename R>
R stack_pop(lua_State * L);

template <typename R>
R cast(lua_State * L);  //!< pretypuje vrchol zasobnika na pozadovany prvok

template <typename T>
bool istype(lua_State * L);  //!< overi, ci na vrchole zasobnika je typ T
//@}


//! Automaticka sprava zasobniku po navrate z volania lua funkcie.
class result
{
public:
	result(result && rhs);
	~result() {flush();}

	int status() const {return _status;}  //!< navratova hodnota call_chunk()
	lua_State * state() const {return _L;}
	void flush();

private:
	result(lua_State * L, int nargs, int status);

	lua_State * _L;
	int _nargs;  //!< number of returned arguments (by function call)
	int _status;

	friend class vm;
};  // result

//! Lua Virtual Machine.
class vm
{
public:
	static vm & default_vm();

	vm(errout luaerr = nullptr);
	~vm() {lua_close(_L);}

	int load_script(char const * fname);
	void register_function(lua_State * L, lua_CFunction, char const * lname);

	result call_function(char const * lname) {return call_function_impl(lname, 0);}

	template <typename T, typename... Args>
	result call_function(char const * lname, T head, Args ... args)
	{
		return call_function_impl(lname, 0, head, args ...);
	}

	int call_function_raw(char const * lname, int narg);  // zavola funkciu bez resultu

	template <typename R>
	R global_variable(char const * name)  //!< Vrati globalnu premennu 'name'.
	{
		assert(!lua_istable(_L, -1) && "not working with tables, use global_variable_raw() instead");
		global_variable_raw(name);
		if (lua_isnil(_L, -1))
			throw std::exception();  // TODO: specify (unknown variable)
		return stack_pop<R>(_L);
	}

	void global_variable_raw(char const * name);  //!< na vrchol zasobnika ulozi globalnu premennu 'name'

	lua_State * state() const {return _L;}

private:
	int report(lua_State * L, int state);  // TODO: odstran
	void report_if_error(int state);
	int call_chunk(lua_State * L, int narg);

	template <typename T, typename... Args>
	result call_function_impl(char const * lname, int nargs, T head, Args ... args)
	{
		stack_push(_L, head);
		return call_function_impl(lname, nargs+1, args ...);
	}

	result call_function_impl(char const * lname, int nargs);  // stop function

	lua_State * _L;
	errout _luaerr;
};  // vm

template <>
inline int stack_at<int>(lua_State * L, int idx)
{
	return lua_tointeger(L, idx);
}

template <>
inline float stack_at<float>(lua_State * L, int idx)
{
	return lua_tonumber(L, idx);
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

template <> 
inline void stack_push<int>(lua_State * L, int const & x)
{
	lua_pushinteger(L, x);
}

template <>
inline void stack_push<float>(lua_State * L, float const & x)
{
	lua_pushnumber(L, x);
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

template <>
inline int cast(lua_State * L)
{
	return lua_tonumber(L, -1);
}

template <>
inline std::string cast(lua_State * L)
{
	return std::string(lua_tostring(L, -1));
}

template <>
inline bool istype<int>(lua_State * L)
{
	return lua_isnumber(L, -1) == 1;
}

template <>
inline bool istype<bool>(lua_State * L)
{
	return lua_isboolean(L, -1) == 1;
}

template <>
inline bool istype<std::string>(lua_State * L)
{
	return lua_isstring(L, -1) == 1;
}


template <typename Value, typename Key>
Value table_value(lua_State * L, Key k, int sidx) 
{
	stack_push(L, k);
	lua_gettable(L, sidx-1);
	return stack_pop<Value>(L);
}


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


//! Iteracia skrz homogennu tabulku.
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


class table_pair
{
public:
	table_pair(lua_State * L) : _L(L) {}

	/*! \return returns lua type
	\saa LUA_TNIL, LUA_TNUMBER, LUA_TBOOLEAN, LUA_TSTRING, LUA_TTABLE, LUA_TFUNCTION, LUA_TUSERDATA, LUA_TTHREAD, and LUA_TLIGHTUSERDATA */
	int key_type() const {return lua_type(_L, -2);}
	int value_type() const {return lua_type(_L, -1);}

	template <typename R = std::string>
	R key() const {return stack_at<R>(_L, -2);}

	template <typename R = std::string>
	R value() const {return stack_at<R>(_L, -1);}

	table_pair * operator->() {return this;}

private:
	lua_State * _L;
};

class table_iterator
	: public std::iterator<std::forward_iterator_tag, table_iterator>
{
public:
	table_iterator(lua_State * L);

	table_iterator & operator++();
	table_pair operator*() const {return table_pair(_L);}
	bool operator==(table_iterator const & rhs) const {return _L == rhs._L;}
	bool operator!=(table_iterator const & rhs) const {return _L != rhs._L;}

private:
	void next();

	lua_State * _L;
};

/*! Iteracia skrz heterogennu tabulku.
\code
	result res = lvm.call_function("some_lua_function");
	for (table_range r(res); r; ++r) {
		if (r->key() == "name")
			cout << "name:" << r->value<string>();
		else if (r->key() == "age")
			cout << "age:" << r->value<int>();
	}
\endcode */
class table_range
{
public:
	table_range(table_iterator beg, table_iterator end) : _it(beg), _end(end) {}
	table_range(result const & res) : table_range(table_iterator(res.state()), table_iterator(nullptr)) {}
	table_range(lua_State * L) : table_range(table_iterator(L), table_iterator(nullptr)) {}

	void operator++() {++_it;}
	table_pair operator*() const {return *_it;}
	table_pair operator->() const {return *_it;}
	operator bool() const {return _it != _end;}
	table_iterator begin() const {return _it;}
	table_iterator end() const {return _end;}

private:
	table_iterator _it, _end;
};

/*! Umožnuje prístup do tabuľky na zásobníku.
\code
	table tbl(L);	
	tbl.at<int>("name") = 123;	
	int name = tbl.at<int>("name");  // name=123
	for (auto kv : tb)
		...
\endcode */
class table
{
public:
	table(lua_State * L) : _L(L) {}
	table(result const & r) : table(r.state()) {}

	template <typename T>
	class fake_ref
	{
	public:
		template <typename U>
		fake_ref(lua_State * L, U const & key) : _L(L), _key_in_stack(true) {stack_push(_L, key);}

		~fake_ref()
		{
			if (_key_in_stack)
				lua_pop(_L, 1);
		}

		operator T() const {return value();}

		template <typename U>
		void operator=(U const & rhs)
		{
			stack_push(_L, rhs);
			lua_settable(_L, -3);  // value, key, table, ... bottom
			_key_in_stack = false;
		}

		void operator=(char const * rhs)
		{
			stack_push(_L, rhs);
			lua_settable(_L, -3);  // value, key, table, ... bottom
			_key_in_stack = false;
		}

		bool operator==(T const & rhs) const {return rhs == value();}
		bool operator!=(T const & rhs) const {return rhs != value();}

	private:
		T value() const
		{
			lua_gettable(_L, -2);
			assert(istype<T>(_L) && "unexpected type");
			T result = lua::cast<T>(_L);
			lua_pop(_L, 1);  // pop result
			_key_in_stack = false;
			return result;
		}

		lua_State * _L;
		mutable bool _key_in_stack;
	};  // fake_ref

	template <typename V, typename K>
	fake_ref<V> at(K const & key) {return fake_ref<V>(_L, key);}

	fake_ref<std::string> at(std::string const & key) {return at<std::string>(key);}

	template <typename K, typename V>
	bool at(K const & key, V & value)
	{
		stack_push(_L, key);
		lua_gettable(_L, -2);

		bool contain = !lua_isnil(_L, -1);
		if (contain)
		{
			assert(istype<V>(_L) && "unexpected type");
			value = stack_at<V>(_L, -1);
		}

		lua_pop(_L, 1);
		return contain;
	}

	table_iterator begin() const {return table_iterator(_L);}
	table_iterator end() const {return table_iterator(nullptr);}

private:
	lua_State * _L;
};  // table

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

