#include <memory>

namespace gl { namespace ui {

class label {};

class button
{
public:
	button();
	~button();
	bool up() const;
	bool down() const;
	void icon(std::shared_ptr<texture2d> t);
	void input();
	void update();
	void render();

	// event interface
	void touch_handler(std::function<bool(ivec2)> func);
	
private:
	int _x, _y, _w, _h;
};

}}  // gl::ui
