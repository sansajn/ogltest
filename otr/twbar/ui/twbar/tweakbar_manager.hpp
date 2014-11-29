#pragma once
#include <vector>
#include <AntTweakBar.h>
#include "core/ptr.hpp"
#include "ui/event_handler.hpp"
#include "tweakbar_handler.hpp"

/*! \ingroup tweakbar */
class tweakbar_manager : public event_handler
{
public:
	struct bar_data 
	{
		tweakbar_manager * owner;
		ptr<tweakbar_handler> bar;
		bool exclusive;
		bool permanent;  //!< true if always active
		char shortcut_key;

		bar_data(ptr<tweakbar_handler> bar, bool exclusive, bool permanent,
			char shortcut_key);

		void active(bool val);
	};
	
	tweakbar_manager(std::vector<bar_data> const & bars, bool minimized = true);
	virtual ~tweakbar_manager();

	void display(double t, double dt) override;
	void reshape(int w, int h) override;
	void idle() override;
	bool mouse_click(button b, state s, modifier m, int x, int y) override;
	bool mouse_motion(int x, int y) override;
	bool mouse_passive_motion(int x, int y) override;
	bool mouse_wheel(wheel b, modifier m, int x, int y) override;
	bool key_typed(unsigned char c, modifier m, int x, int y) override;
	bool key_released(unsigned char c, modifier m, int x, int y) override;
	bool special_key(key k, modifier m, int x, int y) override;
	bool special_key_released(key k, modifier m, int x, int y) override;

protected:
	tweakbar_manager() {}
	void init(std::vector<bar_data> const & bars, bool minimized = true);

private:
	void init_bar();
	void append_bar(bar_data const & b);
	void reset_states() const;

	TwBar * _twbar;
	std::vector<bar_data> _bars;
	int _minimized;
	bool _updated;
	bool _initialized;
};
