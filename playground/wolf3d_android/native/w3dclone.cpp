// do levelu pridava dvere
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>
#include "gl/window.hpp"
#include "gles2/program_gles2.hpp"
#include "gles2/default_shader_gles2.hpp"
#include "gles2/light_gles2.hpp"
#include "gles2/touch_joystick_gles2.hpp"
#include "gles2/ui/touch_button_gles2.hpp"
#include "androidgl/android_window.hpp"
#include "medkit_world.hpp"
#include "level.hpp"
#include "sound.hpp"
#include "resource.hpp"

using std::vector;
using std::string;
using std::shared_ptr;
using glm::vec2;
using glm::ivec2;
using glm::vec3;
using glm::mat3;
using glm::mat4;
using glm::radians;
using glm::translate;
using gl::camera;
using ui::android_window;
using gles2::shader::program;
using gles2::phong_light;
using gles2::ui::touch::joystick;

using namespace phys;


char const * assets_path = "/sdcard/wolf3";
string const font_path = "/usr/share/fonts/truetype/freefont/FreeMono.ttf";
string const health_sound_path = "sound/health.ogg";
string const door_sound_path = "sound/door.ogg";
string const level_music_path = "sound/Get_Them_Before_They_Get_You.ogg";
//string const skinned_shader_path = "shaders/bump_skinned.glsl";
//string const textured_shared_path = "shaders/textured.glsl";

constexpr unsigned joystick_size = 100;


char const * bump_skinned_shader_source = R"(
	#ifdef _VERTEX_
	const int MAX_JOINTS = 36;

	attribute vec3 position;
	attribute vec2 texcoord;
	attribute vec3 normal;
	attribute vec3 tangent;
	attribute vec4 joints;  // skeleton indices
	attribute vec4 weights;

	uniform mat4 local_to_screen;
	uniform mat4 local_to_camera;
	uniform mat3 normal_to_camera;
	uniform vec3 light_direction;
	uniform mat4 skeleton[MAX_JOINTS];  // kostra, ako zoznam transformacii

	varying vec3 l;
	varying vec3 v;
	varying vec2 uv;

	void main()
	{
		mat4 T_skin =
			skeleton[int(joints.x)] * weights.x +
			skeleton[int(joints.y)] * weights.y +
			skeleton[int(joints.z)] * weights.z +
			skeleton[int(joints.w)] * weights.w;

		vec4 n_skin = T_skin * vec4(normal, 0);
		vec4 t_skin = T_skin * vec4(tangent, 0);

		vec3 n = normal_to_camera * n_skin.xyz;
		vec3 t = normal_to_camera * t_skin.xyz;
		vec3 b = cross(n,t);
		mat3 T_tbn = mat3(t,b,n);

		l = normal_to_camera * light_direction * T_tbn;

		vec4 p = local_to_camera * T_skin * vec4(position, 1);
		v = -p.xyz * T_tbn;

		uv = texcoord;
		gl_Position = local_to_screen * T_skin * vec4(position, 1);
	}
	#endif

	#ifdef _FRAGMENT_
	precision mediump float;
	uniform sampler2D diff_tex;  // diffuse texture
	uniform sampler2D norm_tex;  // normal texture
	uniform sampler2D height_tex;  // height texture
	uniform float light_intensity;  // 1.0
	uniform vec3 light_color;  // vec3(1,1,1)
	uniform vec3 material_ambient;  // vec3(.2,.2,.2)
	uniform float material_shininess;  // 64
	uniform float material_intensity;  // 1.0
	uniform vec2 parallax_scale_bias;  // vec2(.04, -.03)

	varying vec3 l;  // light direction in tangent-space
	varying vec3 v;  // view direction in tangent-space
	varying vec2 uv;

	void main()
	{
		vec3 l_norm = normalize(l);
		vec3 v_norm = normalize(v);
		vec3 n = texture2D(norm_tex, uv).xyz * 2.0 - 1.0;

		float diff = max(dot(n, l_norm), 0.0);

		vec3 r = normalize(-reflect(l_norm, n));
		float spec = pow(max(dot(r, v_norm), 0.0), material_shininess) * material_intensity;

		float h = texture2D(height_tex, uv).r * parallax_scale_bias.x + parallax_scale_bias.y;  // TODO: toto je dobre ?
		vec2 uv_ = uv + h * v_norm.xy;
		vec4 texel = texture2D(diff_tex, uv_);

		gl_FragColor = vec4(
			(material_ambient + (diff+spec)*light_intensity*light_color) * texel.rgb, texel.a);
	}
	#endif
)";


class medkit_pick_listenner : public collision_listener
{
public:
	medkit_pick_listenner(level & lvl, rigid_body_world & world) : _lvl(lvl), _world(world) {}
	void collision_event(btCollisionObject * body0, btCollisionObject * body1) override;

	bool medkit_picked = false;

private:
	unsigned count = 0;
	level & _lvl;
	rigid_body_world & _world;
};

void medkit_pick_listenner::collision_event(btCollisionObject * body0, btCollisionObject * body1)
{
	btCollisionObject * medkit = nullptr, * other = nullptr;
	if (((btCollisionObject *)body0)->getUserIndex() == (int)medkit_world::collision_object_type::medkit)
	{
		medkit = (btCollisionObject *)body0;
		other = (btCollisionObject *)body1;
	}
	else if (((btCollisionObject *)body1)->getUserIndex() == (int)medkit_world::collision_object_type::medkit)
	{
		medkit = (btCollisionObject *)body1;
		other = (btCollisionObject *)body0;
	}

	if (medkit)
		std::cout << "medkit collision #" << ++count << std::endl;

	if (medkit && (other->getUserIndex() == (int)medkit_world::collision_object_type::player))
	{
		_world.native()->removeCollisionObject(medkit);  // vyber lekarnicku zo sveta
		_lvl.remove_medkit(medkit);
		medkit_picked = true;
	}
}


//! otvori dvere po kolizii s hracom
class door_opener_listener : public collision_listener
{
public:
	door_opener_listener(level & lvl) : _lvl(lvl) {}
	void collision_event(btCollisionObject * body0, btCollisionObject * body1) override;

	door_object * door_to_open = nullptr;

private:
	level & _lvl;
};

bool is_player(btCollisionObject * body)
{
	return body->getUserIndex() == (int)medkit_world::collision_object_type::player;
}

bool is_medkit(btCollisionObject * body)
{
	return body->getUserIndex() == (int)medkit_world::collision_object_type::medkit;
}

bool is_door(btCollisionObject * body)
{
	return body->getUserIndex() == (int)medkit_world::collision_object_type::door;
}

void door_opener_listener::collision_event(btCollisionObject * body0, btCollisionObject * body1)
{
	// ak je kolizia hrac -> dvere, potom otvor dvere
	btCollisionObject * door = nullptr;
	btCollisionObject * player = nullptr;

	if (is_player(body0))
		player = body0;

	if (is_player(body1))
		player = body1;

	if (is_door(body0))
		door = body0;

	if (is_door(body1))
		door = body1;

	if (!door || !player)
		return;  // kolizia nie je typu player door

	door_to_open = _lvl.find_door(door);
}


class w3dclone_scene : public android_window
{
public:
	using base = android_window;

	w3dclone_scene(parameters const & params);
	~w3dclone_scene();

	void input(float dt) override;
	void update(float dt) override;
	void display() override;
	// TODO: reshape()

private:
	medkit_world _world;  // fyzika
	level _lvl;
	medkit_pick_listenner _medkit_collision;
	door_opener_listener _door_opener;
	player_object _player;
	crosshair_object _crosshair;
	program _player_prog;
	program _crosshair_prog;
	phong_light _player_light;
	joystick _move, _look;
	ui::touch::button _fire;
//	fps_object<w3dclone_scene> _fps;

	bool _player_view = true;
	bool _debug_physics = false;
};


w3dclone_scene::w3dclone_scene(parameters const & params)
	: base{params}
	, _medkit_collision{_lvl, _world}
	, _door_opener{_lvl}
	, _move{ivec2{joystick_size+50, height()-joystick_size-50}, joystick_size, width(), height()}
	, _look{ivec2{width() - joystick_size-50, height()-joystick_size-50}, joystick_size, width(), height()}
//	, _fps{*this, 2.0f}
{
	std::clog << "w3dclone_scene()" << std::endl;

	_world.add_collision_listener(&_medkit_collision);
	_world.add_collision_listener(&_door_opener);
	_lvl.link_with(_world);

	std::clog << "loading shader programs ..." << std::endl;

	_player_prog.from_memory(bump_skinned_shader_source);
	_player_light = phong_light{normalize(vec3{3,5,3}), vec3{1}, 1.0f};
	std::clog << "player shader loaded" << std::endl;
	
	_crosshair_prog.from_memory(gles2::textured_shader_source);
	std::clog << "crosshair shader loaded" << std::endl;

	_player.init(_lvl.player_position(), aspect_ratio());
	_world.link(_player);
	std::clog << "player initialized" << std::endl;

	int position_a = _player_prog.attribute_location("position");
	int texcoord_a = _player_prog.attribute_location("texcoord");
	int normal_a = _player_prog.attribute_location("normal");
	int tangent_a = _player_prog.attribute_location("tangent");
	int joints_a = _player_prog.attribute_location("joints");
	int weights_a = _player_prog.attribute_location("weights");
	gles2::animated_model & mdl = _player.get_model();
	mdl.attribute_location({position_a, texcoord_a, normal_a, tangent_a, joints_a, weights_a});
	std::clog << "player model loaded" << std::endl;

	// vytvor herny svet
	game_world & game = game_world::ref();
	game._player = &_player;
	game._physics = &_world;
	game._enemies = _lvl.enemies();

	al::device::ref().play_music(path_manager::ref().translate_path(level_music_path));  // pusti podmaz TODO: tu chcem moznost nastavit loop

	glClearColor(0, 0, 0, 1);

	// controls
	shared_ptr<program> button_prog{new program{}};
	button_prog->from_memory(gles2::flat_shader_source);
	_fire.init(ivec2{joystick_size + 150, height()-joystick_size-200}, 50, width(), height(), button_prog);

	std::clog << "w3dclone_scene::w3dclone_scene():done" << std::endl;
}

w3dclone_scene::~w3dclone_scene()
{}

void w3dclone_scene::update(float dt)
{
	base::update(dt);
	_world.update(dt);
	_lvl.update(dt);
	_player.update(dt);
	al::device::ref().update();

	if (_medkit_collision.medkit_picked)
	{
		al::device::ref().play_effect(path_manager::ref().translate_path(health_sound_path));
		_player.heal(25);
		_medkit_collision.medkit_picked = false;
	}

	if (_door_opener.door_to_open)
	{
		_door_opener.door_to_open->open();
		_door_opener.door_to_open = nullptr;
	}

	if (_player.health() == 0)
		std::cout << "!!! player death !!!" << std::endl;

	_fire.update();

//	_fps.update(dt);
}

void w3dclone_scene::display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

//	camera * cam = &_free_view.get_camera();
//	if (_player_view)
//		cam = &_player.get_camera();
	camera * cam = &_player.get_camera();

	_lvl.render(*cam);

	auto VP = cam->view_projection();

	if (_debug_physics)
		_world.debug_render(VP);

	if (_player_view)
	{
		camera * cam = &_player.get_camera();
		// player
		mat4 const & V = cam->view();
		mat4 M = mat4{1};
		M *= inverse(V);
		M *= translate(vec3{0,0,1});
		M = rotate(M, radians(90.0f), vec3{0, 1, 0});
		M = rotate(M, radians(-90.0f), vec3{1, 0, 0});
		mat4 world_to_camera = V;
		mat4 local_to_screen = cam->projection() * world_to_camera * M;
		mat3 normal_to_camera = mat3{inverseTranspose(world_to_camera * M)};

		_player_prog.use();
		_player_prog.uniform_variable("local_to_screen", local_to_screen);
		_player_prog.uniform_variable("local_to_camera", V*M);
		_player_prog.uniform_variable("normal_to_camera", normal_to_camera);
		_player_light.apply(_player_prog);
		_player_prog.uniform_variable("material_ambient", vec3{.2});
		_player_prog.uniform_variable("material_shininess", 64.0f);
		_player_prog.uniform_variable("material_intensity", 1.0f);
		_player_prog.uniform_variable("parallax_scale_bias", vec2{.04, -.03});
		_player_prog.uniform_variable("skeleton", _player.skeleton());

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_DEPTH_BUFFER_BIT);
		_player.render(_player_prog);

		_crosshair_prog.use();
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		local_to_screen = scale(.04f * vec3{1, aspect_ratio(), 1});
		_crosshair.render(_crosshair_prog, local_to_screen);

		glDisable(GL_BLEND);
	}

//	_fps.render();

	_move.render();
	_look.render();
	_fire.render();

	base::display();
}

void w3dclone_scene::input(float dt)
{
	// update joystick input (move to joystick.input(dt, user_input))
	for (ui::touch::finger & f : in.touch.fingers())
	{
		joystick::touch_event te;
		if (f.move())
			te = joystick::touch_event::move;
		else if (f.down())
			te = joystick::touch_event::down;
		else
			te = joystick::touch_event::up;

		_move.touch(f.position, te);
		_look.touch(f.position, te);
	}

	// buttons
	_fire.input(in, dt);

	btMatrix3x3 const & B = _player.transform().getBasis();
	btVector3 right_dir = B.getColumn(0);

	// player, move
	{
		float const speed = 2.0f;
		btVector3 velocity_dir{0,0,0};
		btVector3 forward_dir = -B.getColumn(2);

		if (_move.up())
			velocity_dir += forward_dir;
		if (_move.down())
			velocity_dir -= forward_dir;
		if (_move.left())
			velocity_dir -= right_dir;
		if (_move.right())
			velocity_dir += right_dir;

		velocity_dir.setY(0);  // chcem sa pohybovat iba po rovine
		btVector3 velocity = speed*velocity_dir;  // a konstantnou rychlostou
		_player.velocity(velocity);
	}

	// player, look
	{
		float const angular_speed = .8;  // TODO: linearna zmena nie je prave to prave orechove
		float angle = angular_speed*dt;

		if (_look.up())
		{
			btQuaternion R{right_dir, angle};
			R = R * _player.transform().getRotation();  // composed rotation
			R.normalize();
			_player.body()->setCenterOfMassTransform(btTransform{R, _player.transform().getOrigin()});
		}

		if (_look.down())
		{
			btQuaternion R{right_dir, -angle};
			R = R * _player.transform().getRotation();  // composed rotation
			R.normalize();
			_player.body()->setCenterOfMassTransform(btTransform{R, _player.transform().getOrigin()});
		}

		if (_look.left())
		{
			btQuaternion R{btVector3{0,1,0}, angle};
			R = R * _player.transform().getRotation();  // composed rotation
			R.normalize();
			_player.body()->setCenterOfMassTransform(btTransform{R, _player.transform().getOrigin()});
		}

		if (_look.right())
		{
			btQuaternion R{btVector3{0,1,0}, -angle};
			R = R * _player.transform().getRotation();  // composed rotation
			R.normalize();
			_player.body()->setCenterOfMassTransform(btTransform{R, _player.transform().getOrigin()});
		}
	}

	{  // player fire
		if (_fire.down())  // shoot
			_player.fire();
	}

	base::input(dt);
}


w3dclone_scene * w = nullptr;


void create(int width, int height)
{
	std::clog << "create(w:" << width << ", h:" << height << ")" << std::endl;
	assert(!w && "scene already created");

	path_manager::ref().root_path(assets_path);

	al::init_sound_system();
	w = new w3dclone_scene{w3dclone_scene::parameters{}.geometry(width, height)};
}

void destroy()
{
	std::clog << "destroy()" << std::endl;
	delete w;
	w = nullptr;
	al::free_sound_system();
}
