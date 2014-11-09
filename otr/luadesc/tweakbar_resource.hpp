#include <string>
#include <limits>
#include <cassert>
#include <vector>
#include <AntTweakBar.h>
#include "resource/resource_manager.hpp"
#include "tweakbar_handler.hpp"

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
	tweakbar_resource(ptr<resource_manager> resman, std::vector<twbar_variable_desc> const & vars);
	virtual ~tweakbar_resource() {}

	void update_bar(TwBar * bar) override;

	struct data
	{
		virtual ~data() {}
	};

private:
	void clear_data();

	std::vector<twbar_variable_desc> _vars;
	std::vector<data *> _datas;
	ptr<resource_manager> _resman;
};

std::vector<twbar_variable_desc> read_tweakbar_as_lua(std::string const & script);
