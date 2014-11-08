#pragma once
#include "render/mesh.hpp"
#include "render/meshbuffers.hpp"
#include "render/module.hpp"
#include "render/program.hpp"
#include "resource/resource_manager.hpp"
#include "scenegraph/sequence_task.hpp"
#include "scenegraph/scenenode.hpp"


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

class module_resource : public resource, public shader::module
{
public:
	module_resource(int version, char const * src) : shader::module(version, src) {}

	module_resource(int version, char const * vertex, char const * fragment)
		: shader::module(version, vertex, fragment) {}
};

class program_resource : public resource, public shader::program
{
public:
	program_resource(ptr<shader::module> m) : shader::program(m) {}
	program_resource(std::vector<ptr<shader::module>> & modules) : shader::program(modules) {}
};

class scene_node_resource : public resource, public scene_node
{
public:
	scene_node_resource(std::string const & name) : scene_node(name) {}
};
