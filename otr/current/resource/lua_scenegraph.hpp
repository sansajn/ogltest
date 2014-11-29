/* implementuje zdroje scenegraphu v lua formate */
#pragma once
#include <set>
#include <map>
#include <vector>
#include <string>
#include "luatools/luatools.hpp"
#include "render/module.hpp"
#include "taskgraph/task.hpp"
#include "scenegraph/method.hpp"
#include "scenegraph/callmethod_task.hpp"
#include "scenegraph/foreach_task.hpp"
#include "scenegraph/scenenode.hpp"
#include "scenegraph/sequence_task.hpp"
#include "scenegraph/program_task.hpp"
#include "scenegraph/transforms_task.hpp"
#include "scenegraph/drawmesh.hpp"
#include "resource/resource.hpp"
#include "resource/lua_resource.hpp"

class resource_manager;

class lua_node_resource : public lua_resource<scene_node>
{
public:
	lua_node_resource(resource_descriptor * desc, resource_manager * resman);
};

class lua_sequence_resource : public resource_template<sequence_task_factory>
{
public:
	lua_sequence_resource(resource_descriptor * desc, resource_manager * resman);
};

class lua_foreach_factory_resource : public lua_resource<foreach_task_factory>
{
public:
	lua_foreach_factory_resource(resource_descriptor * desc, resource_manager * resman);
};

class lua_callmethod_factory_resource : public resource_template<call_method_task_factory>
{
public:
	lua_callmethod_factory_resource(resource_descriptor * desc, resource_manager * resman);
};

class lua_setprogram_resource : public resource_template<program_task_factory>
{
public:
	lua_setprogram_resource(resource_descriptor * desc, resource_manager * resman);
};

class lua_transforms_resource : public lua_resource<transforms_task_factory>
{
public:
	lua_transforms_resource(resource_descriptor * desc, resource_manager * resman);
};

class lua_drawmesh_resource : public lua_resource<draw_mesh_task_factory>
{
public:
	lua_drawmesh_resource(resource_descriptor * desc, resource_manager * resman);
};

class lua_module_resource : public resource_template<shader::module>
{
public:
	lua_module_resource(resource_descriptor * desc, resource_manager * resman);
};

class lua_program_resource : public lua_resource<shader::program>
{
public:
	lua_program_resource(resource_descriptor * desc, resource_manager * resman);
};

class lua_mesh_resource : public resource_template<mesh_buffers>
{
public:
	lua_mesh_resource(resource_descriptor * desc, resource_manager * resman);
};
