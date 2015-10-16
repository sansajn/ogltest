#pragma once
#include <glm/matrix.hpp>
#include <bullet/LinearMath/btIDebugDraw.h>
#include "program.hpp"
#include "camera.hpp"

// TODO: handluj farby DefaultColors
// TODO: make no copyable
class debug_drawer : public btIDebugDraw
{
public:
	debug_drawer();
	void update(gl::camera & c);  // TODO: nestaci iba transformacia ?

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
	shader::program _solid;
	int _mode;
};
