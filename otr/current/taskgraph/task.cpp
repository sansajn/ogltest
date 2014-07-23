#include "taskgraph/task.h"
#include "scenegraph/scene.h"

task_factory::qualified_name::qualified_name(std::string const & n)
{
	auto dotpos = n.find('.');
	if (dotpos != std::string::npos)
	{
		target = n.substr(0, dotpos);
		name = n.substr(dotpos+1);
	}
	else
		name = n;
}

ptr<scene_node> task_factory::qualified_name::target_node(ptr<scene_node> context)
{
	if (target.empty())
		return nullptr;
	else if (target == "this")
		return context;
	else if (target[0] == '$')
		return context->owner()->node_variable(target.substr(1));
	else
	{
		auto r = context->owner()->nodes(name);
		return !r.empty() ? r.begin()->second : nullptr;
	}
}
