#include "player.hpp"
#include "geometry/utility.hpp"
#include "sound.hpp"
#include "enemy.hpp"
#include "gl/shapes.hpp"
#include "gles2/texture_loader_gles2.hpp"
#include "gles2/model_loader_gles2.hpp"
#include "resource.hpp"

#include <iostream>  // TODO: debug

using std::vector;
using std::string;
using std::max;
using std::min;
using glm::ivec2;
using glm::vec3;
using glm::mat3;
using glm::quat;
using glm::mat3_cast;
using glm::radians;
using gl::camera;
using gles2::shader::program;
using gles2::skeletal_animation;
using gles2::animated_model;
using gles2::animated_model_from_file;
using gles2::model_loader_parameters;
using gles2::material_property;
using geom::right;
using geom::forward;

using namespace phys;

// quake 4 blaster
string const model_path = "blaster/view.md5mesh";

string const anim_paths[] = {
	"blaster/big_recoil.md5anim",
	"blaster/charge_up.md5anim",
	"blaster/fire.md5anim",
	"blaster/fire2.md5anim",
	"blaster/flashlight.md5anim",
	"blaster/idle.md5anim",
	"blaster/lower.md5anim",
	"blaster/raise.md5anim"
};

string const effect_paths[] = {
	"sound/fire01.ogg"
};

char const * crosshair_path = "textures/crosshair_blaster.png";


player_object::player_object()
{
	std::clog << "player_object::player_oject()" << std::endl;
}

void player_object::init(glm::vec3 const & position, float aspect_ratio)
{
	std::clog << "player_object::init()" << std::endl;

	path_manager & pman = path_manager::ref();

	auto model_params = model_loader_parameters{};
	model_params.diffuse_texture_postfix = "_d";
	model_params.ignore_height_texture = true;  // TODO: pri citani height textury to zamrzne
	_mdl = animated_model_from_file(pman.translate_path(model_path), model_params);
	std::clog << "  model loaded" << std::endl;

	for (string const & anim_path : anim_paths)
		_mdl.append_animation(skeletal_animation{pman.translate_path(anim_path)});
	std::clog << "  animation loaded" << std::endl;

	_mdl.animation_sequence(vector<unsigned>{idle_animation});
	_mdl.append_global(new material_property{vec3{.45}, 1.0f, 48.0f});

	float const fovy = radians(70.0f);
	float const near = 0.01f;
	float const far = 1000.0f;
	_cam = camera{fovy, aspect_ratio, near, far};

	float const mass = 90.0f;
	_collision = body_object{make_box_shape(btVector3{0.25, 0.25, 0.25}), mass, btVector3{0, 0.5, 0} + bullet_cast(position)};
	_collision.native()->setAngularFactor(0);  // nechcem aby sa hrac otacal pri kolizii
	_collision.native()->setActivationState(DISABLE_DEACTIVATION);

	std::clog << "player_object::init():done" << std::endl;
}

void player_object::link_with(rigid_body_world & world, int mark)
{
	world.link(_collision);
	_collision.native()->setGravity(btVector3{0,0,0});  // turn off object gravity
	if (mark != -1)
		_collision.native()->setUserIndex(mark);
}

void player_object::input(float dt)
{}

void player_object::update(float dt)
{
	_cam.position = glm_cast(_collision.position());
	_cam.rotation = glm_cast(_collision.rotation());
	_state.update(dt, this);
	_mdl.update(dt);
}

void player_object::render(program & prog)
{
	_mdl.render(prog);
}

void player_object::fire()
{
	_state.enter_fire_sequence();
}

void player_object::damage(unsigned amount)
{
	_health = max(0u, _health - amount);
}

void player_object::heal(unsigned amount)
{
	_health = min(100u, _health + amount);
}

btVector3 const & player_object::velocity() const
{
	return _collision.native()->getLinearVelocity();
}

void player_object::velocity(btVector3 const & v)
{
	_collision.native()->setLinearVelocity(v);
}

player_state_machine::player_state_machine() : state_machine{player_states::idle}
{
	fill_states();
}

void player_state_machine::enter_fire_sequence()
{
	if (current_state() == player_states::idle)
		enqueue_state(player_states::fire);
}

player_state_machine_state & player_state_machine::to_ref(state_descriptor s)
{
	return *_states[(int)s];
}

void player_state_machine::fill_states()
{
	_states[(int)player_states::idle] = &_idle;
	_states[(int)player_states::fire] = &_fire;
}

void player_idle::enter(player_object * p)
{
	p->get_model().animation_sequence(vector<unsigned>{player_object::idle_animation}, animated_model::repeat_mode::loop);
}

player_states player_idle::update(float dt, player_object * p)
{
	return player_states::invalid;
}

inline btVector3 forward(btMatrix3x3 const & M)
{
	return M.getColumn(2);
}

void player_fire::enter(player_object * p)
{
	_t = 0;
	p->get_model().animation_sequence(vector<unsigned>{player_object::fire_animation}, animated_model::repeat_mode::once);  // TODO: dynamicka alokacia (pomale), TODO: specializovana funkcia pre sekvencie dlzky 1
	al::device::ref().play_effect(path_manager::ref().translate_path(effect_paths[player_object::fire1_sfx]));

	// vystrel na ciel
	game_world & game = game_world::ref();
	phys::rigid_body_world * world = game.physics();
	btTransform const & player_transf = game.player()->transform();
	btVector3 from = player_transf.getOrigin();
	btVector3 to = from + (-100.0f * forward(player_transf.getBasis()));
	btCollisionWorld::ClosestRayResultCallback rayres{from, to};
	world->native()->rayTest(from, to, rayres);
	if (rayres.hasHit() && rayres.m_collisionObject->getUserIndex() == (int)game_world::object_types::enemy)  // trafil som nepriatela
	{
		enemy_object & e = game.get_enemy(rayres.m_collisionObject);
		e.damage(10);   // TODO: znahodni hodnotu poskodenia
		std::cout << "enemy id:" << e._id << " was shoot, health:" << e.health() << std::endl;
	}
}

player_states player_fire::update(float dt, player_object * p)
{
	_t += dt;
	if (_t >= DURATION)
		return player_states::idle;

	return player_states::invalid;
}


crosshair_object::crosshair_object()
{
	std::clog << "crosshair_object::crosshair_object()" << std::endl;
	_quad = gl::make_quad_xy<gles2::mesh>();
	_tex = gles2::texture_from_file(path_manager::ref().translate_path(crosshair_path),
		gles2::texture::parameters().filter(gles2::texture_filter::nearest));
}

void crosshair_object::render(program & p, glm::mat4 const & local_to_screen)
{
	p.uniform_variable("local_to_screen", local_to_screen);
	_tex.bind(0);
	p.uniform_variable("s", 0);
	_quad.attribute_location({p.attribute_location("position"), p.attribute_location("texcoord")});  // TODO: ak nemam k dispozicii program pri vytvoreni meshu, potom musim nastavit atributy
	_quad.render();
}
