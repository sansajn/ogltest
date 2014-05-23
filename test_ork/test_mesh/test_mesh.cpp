#include "ork/resource/XMLResourceLoader.h"
#include "ork/resource/ResourceManager.h"
#include "ork/render/FrameBuffer.h"
#include "ork/ui/GlutWindow.h"

using namespace ork;
using namespace std;

/*
// based on 'ork/examples/ResourceExample.cpp'
class quad_view
	: public GlutWindow
{
public:
	quad_view();
	void redisplay(double t, double dt);
	void reshape(int x, int y);
	void idle(bool damaged)	{GlutWindow::idle(damaged);}
	bool keyTyped(unsigned char c, modifier m, int x, int y) {return true;}
	bool mouseWheel(key k, modifier m, int x, int y) {return true;}
	bool mousePassiveMotion(int x, int y) {return true;}
	bool specialKey(key k, modifier m, int x, int y) {return true;}

	static static_ptr<Window> app;

private:
	ptr<ResourceManager> resman;
	ptr<MeshBuffers> globe;
	ptr<MeshBuffers> cube;
	ptr<MeshBuffers> plane;
	ptr<Program> p1;
	ptr<Program> p2;
	ptr<Uniform3f> world_camera;
	ptr<UniformMatrix4f> local_to_world1;
	ptr<UniformMatrix4f> local_to_screen1;
	ptr<UniformMatrix4f> local_to_world2;
	ptr<UniformMatrix4f> local_to_screen2;
	ptr<FrameBuffer> fb;
	float fov;
	float alpha;
	float theta;
	float dist;	
};

static_ptr<Window> quad_view::app;


quad_view::quad_view()
	: GlutWindow(Window::Parameters().size(1024, 768).depth(true))
{
	ptr<XMLResourceLoader> loader = new XMLResourceLoader();
	loader->addPath("./meshes");
	loader->addPath("./shaders");
	loader->addPath("./textures");

	resman = new ResourceManager(loader);
	
	cube = resman->loadResource("quad.mesh").cast<MeshBuffers>();
	plane = resman->loadResource("cube.mesh").cast<MeshBuffers>();
	globe = resman->loadResource("plane.mesh").cast<MeshBuffers>();
	
	p1 = resman->loadResource("camera;spotlight;plastic;").cast<Program>();
	p2 = resman->loadResource(
		"camera;spotlight;texturedPlastic;").cast<Program>();

	world_camera = p1->getUniform3f("worldCameraPos");
	local_to_world1 = p1->getUniformMatrix4f("localToWorld");
	local_to_screen1 = p1->getUniformMatrix4f("localToScreen");
	local_to_world2 = p2->getUniformMatrix4f("localToWorld");
	local_to_screen2 = p2->getUniformMatrix4f("localToScreen");

	fb = FrameBuffer::getDefault();
	fb->setDepthTest(true, LESS);
}

void quad_view::redisplay(double t, double dt)
{
	fb->clear(true, false, true);

	mat4f camera_to_world = mat4f::rotatex(90);
	camera_to_world = camera_to_world * mat4f::rotatey(-alpha);
	camera_to_world = camera_to_world * mat4f::rotatex(-theta);
	camera_to_world = camera_to_world * mat4f::translate(
		vec3f(0.0, 0.0, dist));

	mat4f world_to_camera = camera_to_world.inverse();

	vec4<GLint> vp = fb->getViewport();
	float width = vp.z;
	float height = vp.w;
	float vfov = degrees(2*atan(height/width * tan(radians(fov/2))));
	mat4f camera_to_screen = mat4f::perspectiveProjection(vfov, width/height,
		0.1f, 1e5f);

	world_camera->set(camera_to_world * vec3f::ZERO);

	local_to_world1->setMatrix(mat4f::rotatez(15));
	local_to_screen1->setMatrix(
		camera_to_screen * world_to_camera * mat4f::rotatez(15));
	fb->draw(p1, *cube, cube->mode, 0, cube->nvertices, 1, cube->nindices);

	local_to_world2->setMatrix(
		mat4f::translate(vec3f(0.0, 0.0, -2.0)) * mat4f::rotatez(180));
	local_to_screen2->setMatrix(camera_to_screen * world_to_camera * 
		mat4f::translate(vec3f(0.0, 0.0, -2.0)) * mat4f::rotatez(180));
	fb->draw(p2, *plane, plane->mode, 0, plane->nvertices, 1, plane->nindices);

	GlutWindow::redisplay(t, dt);
}

void quad_view::reshape(int x, int y)
{
	fb->setViewport(vec4<GLint>(0, 0, x, y));
	fb->setDepthTest(true, LESS);
	GlutWindow::reshape(x, y);
	idle(false);
}

int main(int argc, char * argv[])
{
	atexit(Object::exit);
	quad_view::app = new quad_view();
	quad_view::app->start();
	return 0;
}
*/

class ResourceExample : public GlutWindow
{
public:
	ResourceExample(const string &dir) :
		GlutWindow(Window::Parameters().size(1024, 768).depth(true)), fov(80.0), alpha(135), theta(45), dist(15)
	{
		/*
		  FileLogger::File *out = new FileLogger::File("log.html");
		  Logger::INFO_LOGGER = new FileLogger("INFO", out, Logger::INFO_LOGGER);
		  Logger::WARNING_LOGGER = new FileLogger("WARNING", out, Logger::WARNING_LOGGER);
		  Logger::ERROR_LOGGER = new FileLogger("ERROR", out, Logger::ERROR_LOGGER);
*/
		ptr<XMLResourceLoader> resLoader = new XMLResourceLoader();
		resLoader->addPath(dir + "/textures");
		resLoader->addPath(dir + "/shaders");
		resLoader->addPath(dir + "/meshes");

		resManager = new ResourceManager(resLoader);

		cube = resManager->loadResource("cube.mesh").cast<MeshBuffers>();
		plane = resManager->loadResource("plane.mesh").cast<MeshBuffers>();
		globe = resManager->loadResource("quad.mesh").cast<MeshBuffers>();

		p1 = resManager->loadResource("camera;spotlight;plastic;").cast<Program>();
		p2 = resManager->loadResource("camera;spotlight;texturedPlastic;").cast<Program>();

		worldCamera = p1->getUniform3f("worldCameraPos");
		localToWorld1 = p1->getUniformMatrix4f("localToWorld");
		localToScreen1 = p1->getUniformMatrix4f("localToScreen");
		localToWorld2 = p2->getUniformMatrix4f("localToWorld");
		localToScreen2 = p2->getUniformMatrix4f("localToScreen");

		fb = FrameBuffer::getDefault();
		fb->setDepthTest(true, LESS);
	}

	void redisplay(double t, double dt)
	{
		fb->clear(true, false, true);

		mat4f cameraToWorld = mat4f::rotatex(90);
		cameraToWorld = cameraToWorld * mat4f::rotatey(-alpha);
		cameraToWorld = cameraToWorld * mat4f::rotatex(-theta);
		cameraToWorld = cameraToWorld * mat4f::translate(vec3f(0.0, 0.0, dist));

		mat4f worldToCamera = cameraToWorld.inverse();

		vec4<GLint> vp = fb->getViewport();
		float width = (float) vp.z;
		float height = (float) vp.w;
		float vfov = degrees(2 * atan(height / width * tan(radians(fov / 2))));
		mat4f cameraToScreen = mat4f::perspectiveProjection(vfov, width / height, 0.1f, 1e5f);

		worldCamera->set(cameraToWorld * vec3f::ZERO);

		/*localToWorld1->setMatrix(mat4f::rotatez(15));
		localToScreen1->setMatrix(cameraToScreen * worldToCamera * mat4f::rotatez(15));
		fb->draw(p1, *cube, cube->mode, 0, cube->nvertices, 1, cube->nindices);*/

		localToWorld2->setMatrix(mat4f::translate(vec3f(0.0, 0.0, -2.0)) * mat4f::rotatez(180));
		localToScreen2->setMatrix(cameraToScreen * worldToCamera * mat4f::translate(vec3f(0.0, 0.0, -2.0)) * mat4f::rotatez(180));
		fb->draw(p2, *plane, plane->mode, 0, plane->nvertices, 1, plane->nindices);

		localToWorld1->setMatrix(mat4f::rotatez(15));
		localToScreen1->setMatrix(cameraToScreen * worldToCamera * mat4f::rotatez(15));
		fb->draw(p1, *globe, globe->mode, 0, globe->nvertices, 1, globe->nindices);

		GlutWindow::redisplay(t, dt);

/*
		  if (Logger::ERROR_LOGGER != NULL) {
				Logger::ERROR_LOGGER->flush();
		  }
*/
	}

	void reshape(int x, int y)
	{
		fb->setViewport(vec4<GLint>(0, 0, x, y));
		fb->setDepthTest(true, LESS);
		GlutWindow::reshape(x, y);
		idle(false);
	}

	void idle(bool damaged)
	{
		GlutWindow::idle(damaged);
		if (damaged) {
			resManager->updateResources();
		}
	}

	bool keyTyped(unsigned char c, modifier m, int x, int y)
	{
		if (c == 27) {
			::exit(0);
		}
		return true;
	}

	bool mouseWheel(wheel b, modifier m, int x, int y)
	{
		dist = b == WHEEL_UP ? dist * 1.05f : dist / 1.05f;
		return true;
	}

	/*
	 bool mousePassiveMotion(int x, int y)
	 {
		  alpha = float(x) / getWidth() * 360.0;
		  theta = (float(y) / getHeight() - 0.5) * 180.0;
		  return true;
	 }
*/

	bool specialKey(key k, modifier m, int x, int y)
	{
		switch (k) {
		case KEY_F5:
			resManager->updateResources();
			break;
		default:
			break;
		}
		return true;
	}

	static static_ptr<Window> app;

private:
	ptr<ResourceManager> resManager;
	ptr<MeshBuffers> cube;
	ptr<MeshBuffers> plane;
	ptr<MeshBuffers> globe;
	ptr<Program> p1;
	ptr<Program> p2;
	ptr<Uniform3f> worldCamera;
	ptr<UniformMatrix4f> localToWorld1;
	ptr<UniformMatrix4f> localToScreen1;
	ptr<UniformMatrix4f> localToWorld2;
	ptr<UniformMatrix4f> localToScreen2;
	ptr<FrameBuffer> fb;
	float fov;
	float alpha;
	float theta;
	float dist;
};

static_ptr<Window> ResourceExample::app;


int main(int argc, char* argv[])
{
//	 assert(argc > 2);
	 atexit(Object::exit);
	 ResourceExample::app = new ResourceExample(string("."));
	 ResourceExample::app->start();
	 return 0;
}
