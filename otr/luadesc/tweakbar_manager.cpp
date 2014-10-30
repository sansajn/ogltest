#include "tweakbar_manager.hpp"
#include <cassert>

using std::string;


static void TW_CALL ActivateHandlerCallback(void const * value, void * data)
{
	tweakbar_manager::bar_data * b = static_cast<tweakbar_manager::bar_data *>(data);
	b->active(*(bool *)value);
}

static void TW_CALL GetHandlerStatusCallback(void * value, void * data)
{
	tweakbar_manager::bar_data * b = static_cast<tweakbar_manager::bar_data *>(data);
	 *static_cast<bool *>(value) = b->bar->active();
}

tweakbar_manager::bar_data::bar_data(ptr<tweakbar_handler> bar, bool exclusive, bool permanent, char shortcut_key)
	: bar(bar), exclusive(exclusive), permanent(permanent), shortcut_key(shortcut_key)
{	
	assert(!exclusive || !permanent);  // oba nemozu byt naraz aktivne
}

void tweakbar_manager::bar_data::active(bool active_flag)
{
	assert(owner && "owner not set");

	if (exclusive && active_flag)
		owner->reset_states();

	bar->active(active_flag);
	owner->init_bar();
}

tweakbar_manager::tweakbar_manager(std::vector<bar_data> const & bars, bool minimized)
	: _twbar(nullptr), _bars(bars), _minimized(1), _initialized(false), _updated(true)
{
	for (auto & b : _bars)
		b.owner = this;
	TwInit(TW_OPENGL, nullptr);
}

tweakbar_manager::~tweakbar_manager()
{
	if (_twbar)
		TwDeleteBar(_twbar);
	TwTerminate();  // toto da tole cele twbar api
}

void tweakbar_manager::init_bar()
{
	if (_twbar)
		TwRemoveAllVars(_twbar);
	else
	{
		_twbar = TwNewBar("SelectHandlerBar");
		TwSetParam(_twbar, nullptr, "iconified", TW_PARAM_INT32, 1, &_minimized);
	}

	TwSetParam(_twbar, nullptr, "label", TW_PARAM_CSTRING, 1, "Toggle Editors");
	TwSetParam(_twbar, nullptr, "visible", TW_PARAM_CSTRING, 1, "true");

	bool separator = false;
	for (auto const & b : _bars)  // pre bez flagu
	{
		if (b.permanent || b.exclusive)
			continue;
		append_bar(b);
		separator = true;
	}

	for (auto const & b : _bars)  // pre exkluzivne
	{
		if (b.permanent || !b.exclusive)
			continue;
		append_bar(b);
		separator = true;
	}

	if (separator)
		TwAddSeparator(_twbar, nullptr, nullptr);

	for (auto const & b : _bars)
		if (b.bar->active())
			b.bar->update_bar(_twbar);

	_initialized = true;
	_updated = false;
}

void tweakbar_manager::append_bar(bar_data const & b)
{
	string text = b.bar->name() + "Handler";
	string def = b.shortcut_key ? string("key=") + b.shortcut_key : string("");
	TwAddVarCB(_twbar, text.c_str(), TW_TYPE_BOOL32, ActivateHandlerCallback, GetHandlerStatusCallback, (void *)&b, def.c_str());
	TwSetParam(_twbar, text.c_str(), "label", TW_PARAM_CSTRING, 1, b.bar->name().c_str());
}

void tweakbar_manager::reset_states() const
{
	for (auto const & b : _bars)
		if (b.exclusive)
			b.bar->active(false);
}

void tweakbar_manager::display(double t, double dt)
{
	if (!_initialized || _updated)
		init_bar();

	for (auto & b : _bars)
	{
		if (b.bar->active())
		{
			bool need_update = false;
			b.bar->display(need_update);
			_updated |= need_update;
		}
	}
}

void tweakbar_manager::reshape(int w, int h)
{
	TwWindowSize(w, h);
	for (auto & b : _bars)
	{
		if (b.bar->active())
		{
			bool need_update = false;
			b.bar->reshape(w, h, need_update);
			_updated |= need_update;
		}
	}
}

void tweakbar_manager::idle()
{
	for (auto & b : _bars)
	{
		if (b.bar->active())
		{
			bool need_update = false;
			b.bar->idle(need_update);
			_updated |= need_update;
		}
	}
}

bool tweakbar_manager::mouse_click(button b, state s, int x, int y)
{
	if (TwGetBarCount() > 1)
	{
		TwMouseMotion(x, y);

		TwMouseAction action = (s == state::down) ? TW_MOUSE_PRESSED : TW_MOUSE_RELEASED;
		switch (b)
		{
			case button::left:
				if (TwMouseButton(action, TW_MOUSE_LEFT))
					return true;

			case button::middle:
				if (TwMouseButton(action, TW_MOUSE_MIDDLE))
					return true;

			case button::right:
				if (TwMouseButton(action, TW_MOUSE_RIGHT))
					return true;

			default:
				break;  // TODO: support wheel events
		}
	}

	for (auto & bd : _bars)
	{
		if (bd.bar->active())
		{
			bool need_update = false;
			bool handled = bd.bar->mouse_click(b, s, x, y, need_update);
			_updated |= need_update;
			if (handled)
				return true;
		}
	}

	return false;  // not handled
}

bool tweakbar_manager::mouse_motion(int x, int y)
{
	if (TwGetBarCount() > 1)
		if (TwMouseMotion(x, y))
			return true;

	for (auto & b : _bars)
	{
		if (b.bar->active())
		{
			bool need_update = false;
			bool handled = b.bar->mouse_motion(x, y, need_update);
			_updated |= need_update;
			if (handled)
				return true;
		}
	}

	return false;
}

bool tweakbar_manager::mouse_passive_motion(int x, int y)
{
	if (TwGetBarCount() > 1)
		if (TwMouseMotion(x, y))
			return true;

	for (auto & b : _bars)
	{
		if (b.bar->active())
		{
			bool need_update = false;
			bool handled = b.bar->mouse_passive_motion(x, y, need_update);
			_updated |= need_update;
			if (handled)
				return true;
		}
	}

	return false;
}

bool tweakbar_manager::mouse_wheel(wheel b, int x, int y)
{
	// TODO: tweak wheel support

	for (auto & bd : _bars)
	{
		if (bd.bar->active())
		{
			bool need_update = false;
			bool handled = bd.bar->mouse_wheel(b, x, y, need_update);
			_updated |= need_update;
			if (handled)
				return true;
		}
	}
	return false;
}

bool tweakbar_manager::key_typed(unsigned char c, int x, int y)
{
	if (TwGetBarCount() > 1)
	{
		int kmod = 0;
		// TODO: modifier support
		if (TwKeyPressed(c, kmod))
			return true;
	}

	for (auto & b : _bars)
	{
		if (b.bar->active())
		{
			bool need_update = false;
			bool handled = b.bar->key_typed(c, x, y, need_update);
			_updated |= need_update;
			if (handled)
				return true;
		}
	}

	return false;
}

bool tweakbar_manager::key_released(unsigned char c, int x, int y)
{
	for (auto & b : _bars)
	{
		if (b.bar->active())
		{
			bool need_update = false;
			bool handled = b.bar->key_released(c, x, y, need_update);
			_updated |= need_update;
			if (handled)
				return true;
		}
	}
	return false;
}

bool tweakbar_manager::special_key(key k, int x, int y)
{
	// TODO: special key support
	for (auto & b : _bars)
	{
		if (b.bar->active())
		{
			bool need_update = false;
			bool handled = b.bar->special_key(k, x, y, need_update);
			_updated |= need_update;
			if (handled)
				return true;
		}
	}
	return false;
}

bool tweakbar_manager::special_key_released(key k, int x, int y)
{
	for (auto & b : _bars)
	{
		if (b.bar->active())
		{
			bool need_update = false;
			bool handled = b.bar->special_key_released(k, x, y, need_update);
			_updated |= need_update;
			if (handled)
				return true;
		}
	}
	return false;
}
