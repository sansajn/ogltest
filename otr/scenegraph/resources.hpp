#pragma once
#include "render/mesh.hpp"
#include "render/meshbuffers.hpp"
#include "render/module.hpp"
#include "render/program.hpp"
#include "resource/resource_manager.hpp"
#include "scenegraph/sequence_task.hpp"


class mesh_buffers_resource : public resource, public mesh_buffers
{
public:
	mesh_buffers_resource() {}
};

class sequence_task_factory_resource : public resource, public sequence_task_factory
{
public:
	sequence_task_factory_resource(std::vector<ptr<task_factory>> const & subtasks)
		: sequence_task_factory(subtasks)
	{}
};

class module_resource : public resource, public module
{
public:
	module_resource(int version, char const * src) : module(version, src) {}

	module_resource(int version, char const * vertex, char const * fragment)
		: module(version, vertex, fragment) {}
};

class program_resource : public resource, public program
{
public:
	program_resource(ptr<module> m) : program(m) {}
	program_resource(std::vector<ptr<module>> & modules) : program(modules) {}
};
