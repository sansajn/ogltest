#pragma once
#include <string>
#include "core/ptr.h"

class scene_node;  // fwd

/*! @ingroup taskgraph */
class task
{
public:
	virtual ~task() {}
	virtual bool run() = 0;	
};

/*! @ingroup taskgraph */
class task_factory
{
public:
	struct qualified_name
	{
		std::string target;
		std::string name;

		//! \param n nazov v tvare <target>.<name>.
		qualified_name(std::string const & n);

		ptr<scene_node> target_node(ptr<scene_node> context);
	};

	virtual ~task_factory() {}

	/*! \param context #scene_node ktoremu patry metoda vytvarajuca task. */
	virtual ptr<task> create_task(ptr<scene_node> context) = 0;
};
