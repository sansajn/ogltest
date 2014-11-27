#pragma once
#include <string>
#include <lua.hpp>

namespace lua {

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

inline void stack_push(lua_State * L, char const * x)  //! \note: nie je šablonova špecializacia, kvoly pointru x
{
	lua_pushstring(L, x);
}

template <>
inline void stack_push<bool>(lua_State * L, bool const & x)
{
	lua_pushboolean(L, x ? 1 : 0);
}

template <>
inline short stack_pop<short>(lua_State * L)
{
	short tmp = lua_tointeger(L, -1);
	lua_pop(L, 1);
	return tmp;
}

template <>
inline int stack_pop<int>(lua_State * L)
{
	int tmp = lua_tointeger(L, -1);
	lua_pop(L, 1);
	return tmp;
}

template <>
inline float stack_pop<float>(lua_State * L)
{
	float tmp = lua_tonumber(L, -1);
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
inline short cast(lua_State * L)
{
	return lua_tonumber(L, -1);
}

template <>
inline int cast(lua_State * L)
{
	return lua_tonumber(L, -1);
}

template <>
inline float cast(lua_State * L)
{
	return lua_tonumber(L, -1);
}

template <>
inline double cast(lua_State * L)
{
	return lua_tonumber(L, -1);
}

template <>
inline std::string cast(lua_State * L)
{
	return std::string(lua_tostring(L, -1));
}


template <>
inline bool istype<short>(lua_State * L)
{
	return lua_isnumber(L, -1) == 1;
}

template <>
inline bool istype<int>(lua_State * L)
{
	return lua_isnumber(L, -1) == 1;
}

template <>
inline bool istype<float>(lua_State * L)
{
	return lua_isnumber(L, -1) == 1;
}

template <>
inline bool istype<double>(lua_State * L)
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

}  // lua
