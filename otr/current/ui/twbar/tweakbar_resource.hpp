#pragma once
#include <string>
#include <limits>
#include <cassert>
#include <vector>
#include <AntTweakBar.h>
#include "resource/resource_manager.hpp"
#include "ui/twbar/tweakbar_handler.hpp"

using std::string;

struct twbar_variable_desc  //!< universal tweakbar variable descriptor
{
	TwType type;
	string label;
	string group;
	string help;
	string keyincr;
	string keydecr;
	float min;
	float max;
	float step;
	string path;

	twbar_variable_desc() {min = max = step = std::numeric_limits<float>::max();}
};

/*! Vytvori mnozinu tweakbar premennych v tweakbare urcenom volajucim manazerom. */
class tweakbar_resource : public tweakbar_handler
{
public:
	tweakbar_resource(std::vector<twbar_variable_desc> const & vars, resource_manager * resman);
	virtual ~tweakbar_resource() {}

	void update_bar(TwBar * bar) override;

	struct data
	{
		virtual ~data() {}
	};

protected:
	tweakbar_resource() : tweakbar_handler("dummy", true) {}
	void init(std::vector<twbar_variable_desc> const & vars, resource_manager * resman);

private:
	void clear_data();

	std::vector<twbar_variable_desc> _vars;
	std::vector<data *> _datas;
	resource_manager * _resman;  // \note pouzije sa pri hladani uniformou
};
