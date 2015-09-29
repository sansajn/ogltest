#pragma once

namespace gl {

class base_scene
{
public:
	virtual void update(float dt) {}
	virtual void render() = 0;
};

}  // gl
