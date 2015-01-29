#pragma once
#include "ptr.hpp"
#include "program.hpp"
#include "math.hpp"
#include "texture.hpp"

class game_object;
class renderer;
class game_scene;
class game_shader;
class engine;

/*! Definuje chovanie objektov hry.
\sa game_object */
class game_component
{
public:
	virtual ~game_component() {}

	virtual void input(float dt) {}
	virtual void update(float dt) {}
	virtual void render(game_shader & shader, renderer & rend) {}

	transform & transformation();
	transform const & transformation() const;

protected:
	void owner(game_object * o) {_owner = o;}
	virtual void append_to_engine(engine & e) {}

	game_object * _owner;

	friend class game_object;
};

/*! Predstavuje objekty v hre. */
class game_object
{
public:
	transform transformation;

	game_object();
	game_object(glm::vec3 const & pos, glm::quat const & rot = glm::quat(1,0,0,0), glm::vec3 const & scale = glm::vec3(1,1,1));
	virtual ~game_object();

	// object prebera vlastnictvo potomka a komponenty
	void append_child(game_object * child);
	void append_component(game_component * comp);

	void input(float dt);
	void update(float dt);
	void render(game_shader & shader, renderer & rend);

	glm::mat4 const & world_to_local() const;
	glm::mat4 const & local_to_world() const {return _local_to_world;}
	glm::mat4 const & local_to_camera() const {return _local_to_camera;}
	glm::mat4 const & local_to_screen() const {return _local_to_screen;}

	game_scene & owner();

	void append_to_engine(engine & e);

protected:
	void update_local_to_world(glm::mat4 const & parent_to_world);
	void update_local_to_camera(glm::mat4 const & world_to_camera, glm::mat4 const & camera_to_screen);

private:
	void owner(game_scene * s);

	std::vector<game_object *> _children;
	std::vector<game_component *> _components;

	glm::mat4 _local_to_parent;
	glm::mat4 _local_to_world;
	glm::mat4 _local_to_camera;
	glm::mat4 _local_to_screen;
	mutable glm::mat4 _world_to_local;  //!< inv of _local_to_world
	mutable bool _world_to_local_up_to_date;

	game_scene * _owner;

	friend class game_scene;
};

class game_scene
{
public:
	game_scene() : _camera(nullptr) {}

	void append_object(game_object * o);
	game_object & root_object() {return _root;}

	game_object * camera_object() const;
	void camera_object(game_object * cam) {_camera = cam;}
	glm::mat4 const & camera_to_screen() const {return _camera_to_screen;}
	void camera_to_screen(glm::mat4 const & m) {_camera_to_screen = m;}
	glm::vec3 camera_position() {return camera_object()->transformation.position;}

	void input(float dt) {_root.input(dt);}
	void update(float dt);
	void render(renderer & rend);

private:
	game_object _root;
	game_object * _camera;
	glm::mat4 _camera_to_screen;
};

class material
{
public:
	material() {}
	material(ptr<texture> diffuse, float specular_intensity = 0.4f, float specular_power = 64.0f);
	material(ptr<texture> diffuse, ptr<texture> normalmap, float specular_intensity = 0.4f, float specular_power = 64.0f);
	material(ptr<texture> diffuse, ptr<texture> normalmap, ptr<texture> heightmap, float specular_intensity = 0.4f, float specular_power = 64.0f, float disp_scale = 0.04f, float disp_bias = -0.03f);

	float get_float(std::string const & name) const;
	glm::vec3 get_vector(std::string const & name) const;
	ptr<texture> get_texture(std::string const & name) const;

	bool find(std::string const & name, float & val) const;
	bool find(std::string const & name, glm::vec3 & val) const;
	bool find(std::string const & name, ptr<texture> & val) const;

	void assoc_float(std::string const & name, float v) {_floats[name] = v;}
	void assoc_vector(std::string const & name, glm::vec3 const & v) {_vectors[name] = v;}
	void assoc_texture(std::string const & name, ptr<texture> t) {_textures[name] = t;}

private:
	std::map<std::string, ptr<texture>> _textures;
	std::map<std::string, float> _floats;
	std::map<std::string, glm::vec3> _vectors;
};

class game_shader
{
public:
	game_shader(std::string const & name) : _prog(name) {}
	virtual ~game_shader() {}
	virtual void update_uniforms(material const & mat, game_object & obj) = 0;

protected:
	shader::program _prog;
};

class renderer  //!< implementuje tzv. forward renderer
{
public:
	renderer();
	~renderer();
	void render(game_object & root);
	void append_light(game_shader * s) {_shaders.push_back(s);}

private:
	std::vector<game_shader *> _shaders;
};

class engine
{
public:
	void append(game_object * obj);
	void update(float dt) {_scene.update(dt);}
	void render();

	void append_light(game_shader * s);
	void camera_object(game_object * cam, glm::mat4 const & projection);

private:
	renderer _rend;
	game_scene _scene;
};
