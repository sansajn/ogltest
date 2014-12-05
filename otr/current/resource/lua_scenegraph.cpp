#include "lua_scenegraph.hpp"
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include "core/utils.hpp"
#include "luatools/luatools.hpp"
#include "luatools/table.hpp"
#include "resource/assimp_loader.hpp"
#include "resource/orkmesh_loader.hpp"
#include "resource/resource_manager.hpp"
#include "resource/resource_factory.hpp"
#include "resource/lua_resource.hpp"
#include "resource/lua_resource_loader.hpp"

using std::string;

lua_node_resource::lua_node_resource(resource_descriptor * desc, resource_manager * resman)
{
	lua_State * L = state(desc);
	lua::table tb(L);  // {type='node', ...}
	assert(tb.at("type") == "node" && "node element expected");

	glm::mat4 ltop;

	for (lua::field_pair fp : tb|lua::only_string_keys)  // attributes
	{
		if (fp.key() == "name")
			name(fp.value());
		else if (fp.key() == "flags")  // 'flag1,flag2,...'
		{
			string flags = fp.value();
			for (string const & s : tokenizer(flags, ","))
				append_flag(s);
		}
	}

	for (lua::field_pair kv : tb|lua::only_number_keys)  // elements
	{
		lua::table tb(L);
		std::string type = tb.at("type");
		if (type == "node")
		{
			ptr<scene_node> child;
			std::string value;
			if (tb.at("value", value))
				child = resman->load_resource<scene_node>(value);
			else
				child = make_ptr<lua_node_resource>(desc, resman);

			if (child)
				append_child(child);
		}
		else if (type == "module")
		{
			ptr<shader::module> m = resman->load_resource<shader::module>(get_variable_name(L, "value"));
			assoc_module(tb.at("id"), m);
		}
		else if (type == "mesh")
		{
			ptr<mesh_buffers> m = resman->load_resource<mesh_buffers>(tb.at("value"));
			assoc_mesh(tb.at("id"), m);
		}		
		else if (type == "method")
		{
			ptr<task_factory> factory = resman->load_resource<task_factory>(get_variable_name(L, "value"));
			if (factory)
			{
				ptr<method> m = make_ptr<method>(factory);

				bool enabled;
				if (tb.at("enabled", enabled))
					m->enable(enabled);

				assoc_method(tb.at("id"), m);
			}
		}
		else if (type == "field")
		{
			ptr<resource> field = resman->load_resource(get_variable_name(L, "value"));
			assoc_field(tb.at("id"), field);
		}
		else if (type == "rotate")
		{
			float angle;
			if (tb.at(1, angle))  // x
				ltop = glm::rotate(ltop, angle, glm::vec3(1, 0, 0));
			if (tb.at(2, angle))  // y
				ltop = glm::rotate(ltop, angle, glm::vec3(0, 1, 0));
			if (tb.at(3, angle))  // z
				ltop = glm::rotate(ltop, angle, glm::vec3(0, 0, 1));
		}  // rotate
		else if (type == "translate")
		{
			float x = tb.field(1, 0.0f);
			float y = tb.field(2, 0.0f);
			float z = tb.field(3, 0.0f);
			ltop = glm::translate(ltop, glm::vec3(x, y, z));
		}
	}  // for

	local_to_parent(ltop);
}

lua_sequence_resource::lua_sequence_resource(resource_descriptor * desc, resource_manager * resman)
{
	auto ldesc = dynamic_cast<lua_resource_descriptor *>(desc);
	lua_State * L = ldesc->L;

	lua::table tb(L);  // {type='sequence', }
	assert(tb.at("type") == "sequence" && "sequence element expected");

	std::vector<ptr<task_factory>> subtasks;

	for (lua::field_pair fp : tb)
	{
		if (fp.key_type() != LUA_TNUMBER)
			continue;
		lua::table tb(L);
		std::string type = tb.at("type");
		lua_resource_descriptor desc(L, type);
		auto t = std::dynamic_pointer_cast<task_factory>(resource_factory::ref().create(desc, *resman));
		if (t)
			subtasks.push_back(t);
		assert(t && "unable to create a task or bad cast");
	}

	init(subtasks);
}

lua_foreach_factory_resource::lua_foreach_factory_resource(resource_descriptor * desc, resource_manager * resman)
{
	lua_State * L = state(desc);

	lua::table tb(L);  // {type='foreach', var=<string>, flag=<string>, [culling=<bool>, parallel=<bool>]}
	assert(tb.at("type") == "foreach" && "foreach element expected");

	std::string var = tb.field("var");
	std::string flag = tb.field("flag");
	bool cull = tb.field("culling", true);
	bool parallel = tb.field("parallel", true);

	ptr<task_factory> subtask;
	for (auto fp : tb|lua::only_number_keys)
	{
		lua::table tb(L);
		std::string type = tb.at("type");
		if (type == "callmethod")
		{
			lua_resource_descriptor desc(L, type);
			subtask = std::dynamic_pointer_cast<task_factory>(resource_factory::ref().create(desc, *resman));
		}
	}

	assert(subtask && "empty foreach loop in lua script");  // TODO: let lua check this

	init(var, flag, cull, parallel, subtask);
}

lua_callmethod_factory_resource::lua_callmethod_factory_resource(resource_descriptor * desc, resource_manager * resman)
{
	auto ldesc = dynamic_cast<lua_resource_descriptor *>(desc);
	lua_State * L = ldesc->L;

	lua::table tb(L);  // {type='callmethod', <qualified-name>}
	assert(tb.at("type") == "callmethod" && "callmethod element expected");

	std::string meth_name = tb.at<std::string>(1);

	init(meth_name);
}

lua_transforms_resource::lua_transforms_resource(resource_descriptor * desc, resource_manager * resman)
{
	lua_State * L = state(desc);

	lua::table tb(L);
	assert(tb.at("type") == "transforms" && "transforms element expected");

	std::string module = tb.field("module");
	std::string local_to_screen = tb.field("local_to_screen");
	std::string world_pos = tb.field("world_pos");

	init(task_factory::qualified_name(module), local_to_screen, world_pos);
}

lua_setprogram_resource::lua_setprogram_resource(resource_descriptor * desc, resource_manager * resman)
{
	auto ldesc = dynamic_cast<lua_resource_descriptor *>(desc);
	lua_State * L = ldesc->L;

	lua::table tb(L);  // {type='setprogram', }
	assert(tb.at("type") == "setprogram" && "setprogram element expected");

	std::vector<task_factory::qualified_name> modules;

	for (lua::field_pair fp : tb|lua::only_number_keys)
	{
		lua::table tb(L);
		std::string type = tb.at("type");
		if (type == "module")
		{
			std::string name;
			if (tb.at("name", name))
				modules.push_back(task_factory::qualified_name(name));
			else
				assert(false && "name attribute expected");
		}
	}

	init(modules, false);
}

lua_drawmesh_resource::lua_drawmesh_resource(resource_descriptor * desc, resource_manager * resman)
{
	lua_State * L = state(desc);

	lua::table tb(L);  // {type='drawmesh', }
	assert(tb.at("type") == "drawmesh" && "drawmesh element expected");

	std::string name = tb.at("name");
	int count = tb.field("count", 1);

	init(name, count);
}

lua_module_resource::lua_module_resource(resource_descriptor * desc, resource_manager * resman)
{
	auto ldesc = dynamic_cast<lua_resource_descriptor *>(desc);
	lua_State * L = ldesc->L;

	lua::table tb(L);  // {type='module', }
	assert(tb.at("type") == "module" && "module element expected");

	int version = tb.field("version", 330);
	std::string source = tb.field("source");
	std::string vertex = tb.field("vertex");
	std::string fragment = tb.field("fragment");
	std::string options = tb.field("options");

	std::string header;
	for (std::string s : tokenizer(options, ","))
		header += "#define " + s + "\n";

	auto loader = std::dynamic_pointer_cast<lua_resource_loader>(resman->loader());
	assert(loader && "invalid loader pointer");

	if (!source.empty())
	{
		std::string content = header + loader->load_text(source);
		init(version, content.c_str());
	}
	else
	{
		std::string vertex_content;
		if (!vertex.empty())
			vertex_content = loader->load_text(vertex);

		std::string fragment_content;
		if (!fragment.empty())
			fragment_content = loader->load_text(fragment);

		char const * h = header.empty() ? nullptr : header.c_str();
		char const * vs = vertex_content.empty() ? nullptr : vertex_content.c_str();
		char const * fs = fragment_content.empty() ? nullptr : fragment_content.c_str();

		init(version, h, vs, h, fs);
	}
}

lua_program_resource::lua_program_resource(resource_descriptor * desc, resource_manager * resman)
{
	lua_State * L = state(desc);

	std::vector<ptr<shader::module>> modules;

	lua::table tb(L);
	std::string module_list = tb.at("modules");
	for (auto tok : tokenizer(module_list, ";"))
	{
		ptr<shader::module> module = resman->load_resource<shader::module>(tok);
		if (module)
			modules.push_back(module);
		else
			throw std::exception();  // TODO: specify (can't create a program)
	}

	init(modules);
}

lua_mesh_resource::lua_mesh_resource(resource_descriptor * desc, resource_manager * resman)
{
	auto ldesc = dynamic_cast<lua_resource_descriptor *>(desc);
	lua_State * L = ldesc->L;

	lua::table tb(L);  // {type='mesh', id=<s>, value=''}
	assert(tb.at("type") == "mesh" && "mesh element expected");

	std::string name = tb.at("value");
	size_t dotpos = name.rfind('.');
	std::string ext = name.substr(dotpos+1);

	auto rloader = std::dynamic_pointer_cast<lua_resource_loader>(resman->loader());

	if (ext == "mesh")
	{
		orkmesh_loader loader;
		loader.load(rloader->find_resource(name), this);
	}
	else
	{
		assimp_loader loader;
		loader.load(rloader->find_resource(name), this);
	}
}
