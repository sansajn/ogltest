#pragma once
#include "resource/lua_resource.hpp"
#include "ui/twbar/tweakbar_manager.hpp"
#include "ui/twbar/tweakbar_resource.hpp"
#include "ui/twbar/draw_tweakbar_task.hpp"

class resource_manager;

class lua_draw_tweakbar_resource : public lua_resource<draw_tweakbar_factory>
{
public:
	lua_draw_tweakbar_resource(resource_descriptor * desc, resource_manager * resman);
};

class lua_tweakbar_manager_resource : public lua_resource<tweakbar_manager>
{
public:
	lua_tweakbar_manager_resource(resource_descriptor * desc, resource_manager * resman);
};

class lua_tweakbar_resource : public lua_resource<tweakbar_resource>
{
public:
	lua_tweakbar_resource(resource_descriptor * desc, resource_manager * resman);
};
