#pragma once
#include <glm/matrix.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include "gles2/program_gles2.hpp"

namespace phys {

class debug_draw_impl : public btIDebugDraw, private boost::noncopyable
{
public:
	debug_draw_impl();
	void update(glm::mat4 const & world_to_screen);
	void setDebugMode(int mode) override {_mode = mode;}
	int getDebugMode() const override {return _mode;}

	void drawContactPoint(btVector3 const & pointOnB, btVector3 const & normalOnB,
		btScalar distance, int lifeTime, btVector3 const & color) override;

	void drawLine(btVector3 const & from, btVector3 const & to, btVector3 const & color) override;
	void reportErrorWarning(char const * warningString) override {}
	void draw3dText(btVector3 const & location, char const * textString) override {}
	void toggle_debug_flag(int flag);

private:
	glm::mat4 _world_to_screen;
	gles2::shader::program _solid;
	int _mode;
};

}   // phys
