// do levelu pridava dvere
#include <vector>
#include <string>
#include <iostream>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>
#include "gl/window.hpp"
#include "gles2/program_gles2.hpp"
#include "gles2/default_shader_gles2.hpp"
#include "gl/controllers.hpp"
#include "medkit_world.hpp"
#include "level.hpp"
#include "sound.hpp"
#include "resource.hpp"

using std::vector;
using std::string;
using std::to_string;
using glm::vec2;
using glm::vec3;
using glm::mat3;
using glm::mat4;
using glm::radians;
using glm::translate;
using gl::camera;
using gl::free_camera;
using ui::glut_pool_window;
using gles2::shader::program;

using namespace phys;

char const * assets_path = "assets";
string const font_path = "/usr/share/fonts/truetype/freefont/FreeMono.ttf";

string const health_sound_path = "assets/sound/health.ogg";
string const door_sound_path = "assets/sound/door.ogg";
string const level_music_path = "assets/sound/03_-_Wolfenstein_3D_-_DOS_-_Get_Them_Before_They_Get_You.ogg";

string const skinned_shader_path = "assets/shaders/bump_skinned.glsl";
string const textured_shared_path = "assets/shaders/textured.glsl";


class medkit_pick_listenner : public collision_listener
{
public:
	medkit_pick_listenner(level & lvl, rigid_body_world & world) : _lvl{lvl}, _world{world} {}
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


class w3dclone_scene : public glut_pool_window
{
public:
	using base = glut_pool_window;

	w3dclone_scene();
	~w3dclone_scene();

	void input(float dt) override;
	void update(float dt) override;
	void display() override;

private:
	medkit_world _world;  // fyzika
	level _lvl;
	medkit_pick_listenner _medkit_collision;
	player_object _player;
	crosshair_object _crosshair;
//	axis_object _axis;
//	light_object _light;
	program _player_prog;
	program _crosshair_prog;
//	fps_object<w3dclone_scene> _fps;

	free_camera<w3dclone_scene> _free_view;
	bool _player_view = true;

	bool _debug_physics = false;
};


char const * bump_skinned_shader_source = R"(
	#ifdef _VERTEX_
	const int MAX_JOINTS = 64;

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
		
		float diff = max(dot(n, l_norm), 0);
		
		vec3 r = normalize(-reflect(l_norm, n));
		float spec = pow(max(dot(r, v_norm), 0), material_shininess) * material_intensity;
		
		float h = texture2D(height_tex, uv).r * parallax_scale_bias.x + parallax_scale_bias.y;  // TODO: toto je dobre ?
		vec2 uv_ = uv + h * v_norm.xy;
		vec4 texel = texture2D(diff_tex, uv_);
		
		gl_FragColor = vec4(
			(material_ambient + (diff+spec)*light_intensity*light_color) * texel.rgb, texel.a);
	}
	#endif
)";


w3dclone_scene::w3dclone_scene()
	: base{parameters{}.name("wolfenstein 3d clone")}
	, _medkit_collision{_lvl, _world}
//	, _fps{*this, 2.0f}
	, _free_view{radians(70.0f), aspect_ratio(), 0.01, 1000, *this}
{
	_world.add_collision_listener(&_medkit_collision);

	_lvl.link_with(_world);

	_player_prog.from_memory(bump_skinned_shader_source);
	_crosshair_prog.from_memory(gles2::textured_shader_source);

	_player.init(_lvl.player_position(), radians(70.0f), aspect_ratio(), 0.01, 1000, this);
	_world.link(_player);

	int position_a = _player_prog.attribute_location("position");
	int texcoord_a = _player_prog.attribute_location("texcoord");
	int normal_a = _player_prog.attribute_location("normal");
//	int tangent_a = _player_prog.attribute_location("tangent");
	int joints_a = _player_prog.attribute_location("joints");
	int weights_a = _player_prog.attribute_location("weights");
	gles2::animated_model & mdl = _player.get_model();
	mdl.attribute_location({position_a, texcoord_a, normal_a, /*tangent_a,*/ joints_a, weights_a});

	// vytvor herny svet
	game_world & game = game_world::ref();
	game._player = &_player;
	game._physics = &_world;
	game._enemies = _lvl.enemies();

//	al::default_device->play_music(level_music_path);  // pusti podmaz TODO: tu chcem loop

	glClearColor(0, 0, 0, 1);
}

w3dclone_scene::~w3dclone_scene()
{}

void w3dclone_scene::update(float dt)
{
	base::update(dt);
	_world.update(dt);
	_lvl.update(dt);
	_player.update(dt);

	if (_medkit_collision.medkit_picked)
	{
		al::default_device->play_effect(health_sound_path);
		_player.heal(25);
		_medkit_collision.medkit_picked = false;
	}

	if (_player.health() == 0)
		std::cout << "!!! player death !!!" << std::endl;

//	_fps.update(dt);
}

void w3dclone_scene::display()
{
	vec3 const light_pos{3,5,3};

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	camera * cam = &_free_view.get_camera();
	if (_player_view)
		cam = &_player.get_camera();

	_lvl.render(*cam);

	auto VP = cam->view_projection();
//	_axis.render(VP);
//	_light.render(VP * translate(light_pos));

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
		_player_prog.uniform_variable("light_intensity", 1.0f);
		_player_prog.uniform_variable("light_direction", normalize(light_pos));
		_player_prog.uniform_variable("light_color", vec3{1});
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

	base::display();
}

void w3dclone_scene::input(float dt)
{
	if (_player_view)
		_player.input(dt);
	else
		_free_view.input(dt);

	if (in.key('e'))  // open door
	{
		door_object * d = _lvl.find_door(_player.body()->getWorldTransform(), _world);
		if (d)
			d->open();
	}

	if (in.key(' '))  // shoot
		_player.fire();

	if (in.key_up('1'))
		_player_view = false;

	if (in.key_up('2'))
		_player_view = true;

	if (in.key_up('g'))
		_debug_physics = (_debug_physics ? false : true);

	base::input(dt);
}


int main(int argc, char * argv[])
{
	al::init_sound_system();

	path_manager::ref().root_path(assets_path);

	w3dclone_scene w;
	w.start();

	al::free_sound_system();
	return 0;
}
