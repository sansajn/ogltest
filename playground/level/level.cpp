#include "level.hpp"
#include <vector>
#include <glm/gtc/matrix_inverse.hpp>
#include <bullet/BulletCollision/CollisionShapes/btBox2dShape.h>

using std::vector;
using std::string;
using std::shared_ptr;
using glm::vec2;
using glm::vec3;
using glm::mat3;
using glm::mat4;
using glm::radians;
using glm::inverseTranspose;
using glm::normalize;
using gl::mesh;
using gl::mesh_from_vertices;
using gl::vertex;
using gl::camera;

// TODO: hardcoded
string level_data_path = "assets/bitmaps/levelTest.png";
string collection_texture_path = "assets/textures/collection.png";

static string solid_shader_source = R"(
	// zobrazi model bez osvetlenia
	uniform mat4 local_to_screen;
	uniform vec3 color = vec3(0.7, 0.7, 0.7);
	#ifdef _VERTEX_
	layout(location=0) in vec3 position;
	void main()	{
		gl_Position = local_to_screen * vec4(position,1);
	}
	#endif
	#ifdef _FRAGMENT_
	out vec4 fcolor;
	void main()	{
		if (gl_FrontFacing)
			fcolor = vec4(0,1,0,1);
		else
			fcolor = vec4(1,0,0,1);
//		fcolor = vec4(color, 1);
	}
	#endif
)";

static string shaded_shader_source = R"(
	// zozbrazi model s tienovanim zalozenom na normale vrchola
	uniform mat4 local_to_screen;
	uniform mat3 normal_to_world;
	uniform vec3 color = vec3(0.7, 0.7, 0.7);
	uniform vec3 light_dir = normalize(vec3(1,1,1));
	#ifdef _VERTEX_
	layout(location=0) in vec3 position;
	layout(location=2) in vec3 normal;
	out vec3 n;
	void main()	{
		n = normal_to_world * normal;
		gl_Position = local_to_screen * vec4(position, 1);
	}
	#endif
	#ifdef _FRAGMENT_
	in vec3 n;
	out vec4 fcolor;
	void main()	{
		vec3 vcolor;
		if (gl_FrontFacing)
			vcolor = vec3(0,1,0);
		else
			vcolor = vec3(1,0,0);
		fcolor = vec4(max(dot(n, light_dir), 0.2) * vcolor, 1);
//		fcolor = vec4(color, 1);
	}
	#endif
)";

string textured_shader_source = R"(
	// zobrazi otexturovany model (bez osvetlenia)
	uniform mat4 local_to_screen;
	uniform sampler2D s;
	#ifdef _VERTEX_
	layout(location=0) in vec3 position;
	layout(location=1) in vec2 texcoord;
	out vec2 uv;
	void main()	{
		uv = texcoord;
		gl_Position = local_to_screen * vec4(position,1);
	}
	#endif
	#ifdef _FRAGMENT_
	in vec2 uv;
	out vec4 fcolor;
	void main() {
		fcolor = texture(s, uv);
	}
	#endif
)";

unsigned bitmap::at(unsigned x, unsigned y) const
{
	using Magick::Quantum;

	Magick::Color c = _im.pixelColor(x, y);
	unsigned result =
		unsigned(c.redQuantum() * (255.0/QuantumRange) + 0.5) << 24 |
		unsigned(c.greenQuantum() * (255.0/QuantumRange) + 0.5) << 16 |
		unsigned(c.blueQuantum() * (255.0/QuantumRange) + 0.5) << 8 |
		unsigned(c.alphaQuantum() * (255.0/QuantumRange) + 0.5);
	return result;
}

level::level()
{
	_data.load(level_data_path);
	generate_level(_data);
	_walls = texture2d{collection_texture_path};
//	_prog.from_memory(shaded_shader_source);
	_prog.from_memory(textured_shader_source);
}

void level::render(camera & c)
{
	vec3 const light_pos{3,5,3};

	mat4 M = mat4{1};
	mat4 local_to_screen = c.view_projection() * M;
	_prog.use();
	_prog.uniform_variable("local_to_screen", local_to_screen);
//	_prog.uniform_variable("normal_to_world", mat3{inverseTranspose(M)});
//	_prog.uniform_variable("light_dir", normalize(light_pos));
	_walls.bind(0);
	_prog.uniform_variable("s", 0);
//	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	_mesh.render();
}

glm::vec3 const & level::player_position() const
{
	return _player_pos;
}

void level::link_with_world(rigid_body_world & world)
{
	for (auto & w : _phys_walls)
		world.add(w.body());
	world.add(_phys_ground.body());
}

void level::generate_level(bitmap const & data)
{
	vector<vertex> vertices;
	vector<unsigned> indices;

	float const c_width = 1.0;
	float const c_height = 1.0;
	float const c_length = 1.0;

	shared_ptr<btCollisionShape> phys_wall_shape{new btBox2dShape{btVector3{0.5, 0.5, 0}}};

	_phys_ground = physics_object{
		shared_ptr<btCollisionShape>(new btBox2dShape{btVector3{data.width(), data.height(), 0}}),
			0,	btVector3{0,0,0}, btQuaternion{btVector3{1,0,0}, radians(-90.0f)}};

	for (int y = 1; y < data.height()-1; ++y)
	{
		for (int x = 1; x < data.width()-1; ++x)
		{
			unsigned cell = data.at(x,y);
			if (cell == 0)
				continue;

			// special
			uint8_t special_val = (cell >> 8) & 0xff;
			if ( special_val == 1)  // player
				_player_pos = vec3{x, 0, -y};

			// floor texture
			unsigned floor_tex = (cell >> 16) & 0xff;
			int tx = floor_tex / 16;
			int ty = floor_tex % 4;
			tx /= 4;
			float x_max = 1 - tx*0.25;
			float x_min = x_max - 0.25;
			float y_max = ty*0.25;
			float y_min = y_max + 0.25;

			// floor
			unsigned size = vertices.size();
			vertices.push_back(vertex{vec3{x*c_width, 0, -y*c_height}, vec2{x_min, y_max}, vec3{0,1,0}});
			vertices.push_back(vertex{vec3{(x+1)*c_width, 0, -y*c_height}, vec2{x_max, y_max}, vec3{0,1,0}});
			vertices.push_back(vertex{vec3{(x+1)*c_width, 0, -(y+1)*c_height}, vec2{x_max, y_min}, vec3{0,1,0}});
			vertices.push_back(vertex{vec3{x*c_width, 0, -(y+1)*c_height}, vec2{x_min, y_min}, vec3{0,1,0}});
			indices.insert(indices.end(), {size, size+1, size+2, size+2, size+3, size});

			// TODO: bez stropu vypada level lepsie
			// ceil
//			size = vertices.size();
//			vertices.push_back(vertex{vec3{x*c_width, c_length, -y*c_height}, vec2{x_min, y_max}, vec3{0,-1,0}});
//			vertices.push_back(vertex{vec3{(x+1)*c_width, c_length, -y*c_height}, vec2{x_max, y_max}, vec3{0,-1,0}});
//			vertices.push_back(vertex{vec3{(x+1)*c_width, c_length, -(y+1)*c_height}, vec2{x_max, y_min}, vec3{0,-1,0}});
//			vertices.push_back(vertex{vec3{x*c_width, c_length, -(y+1)*c_height}, vec2{x_min, y_min}, vec3{0,-1,0}});
//			indices.insert(indices.end(), {size+2, size+1, size, size, size+3, size+2});

			unsigned wall_tex = (cell >> 24) & 0xff;
			tx = wall_tex / 16;
			ty = wall_tex % 4;
			tx /= 4;
			x_max = 1 - tx*0.25;
			x_min = x_max - 0.25;
			y_max = ty*0.25;
			y_min = y_max + 0.25;

			if (data.at(x-1, y) == 0)  // left wall
			{
				size = vertices.size();
				vertices.push_back(vertex{vec3{x*c_width, 0, -y*c_height}, vec2{x_min, y_max}, vec3{1,0,0}});
				vertices.push_back(vertex{vec3{x*c_width, 0, -(y+1)*c_height}, vec2{x_max, y_max}, vec3{1,0,0}});
				vertices.push_back(vertex{vec3{x*c_width, c_length, -(y+1)*c_height}, vec2{x_max, y_min}, vec3{1,0,0}});
				vertices.push_back(vertex{vec3{x*c_width, c_length, -y*c_height}, vec2{x_min, y_min}, vec3{1,0,0}});
				indices.insert(indices.end(), {size, size+1, size+2, size+2, size+3, size});

				_phys_walls.emplace_back(phys_wall_shape, 0, btVector3(x, 0.5, -(y+0.5)*c_height), btQuaternion{btVector3{0,1,0}, radians(270.0f)});
			}

			if (data.at(x+1, y) == 0)  // right wall
			{
				size = vertices.size();
				vertices.push_back(vertex{vec3{(x+1)*c_width, 0, -y*c_height}, vec2{x_min, y_max}, vec3{-1,0,0}});
				vertices.push_back(vertex{vec3{(x+1)*c_width, 0, -(y+1)*c_height}, vec2{x_max, y_max}, vec3{-1,0,0}});
				vertices.push_back(vertex{vec3{(x+1)*c_width, c_length, -(y+1)*c_height}, vec2{x_max, y_min}, vec3{-1,0,0}});
				vertices.push_back(vertex{vec3{(x+1)*c_width, c_length, -y*c_height}, vec2{x_min, y_min}, vec3{-1,0,0}});
				indices.insert(indices.end(), {size, size+3, size+2, size+2, size+1, size});

				_phys_walls.emplace_back(phys_wall_shape, 0, btVector3(x+1, 0.5, -(y+0.5)*c_height), btQuaternion{btVector3{0,1,0}, radians(90.0f)});
			}

			if (data.at(x, y+1) == 0)  // front wall
			{
				size = vertices.size();
				vertices.push_back(vertex{vec3{x*c_width, 0, -(y+1)*c_height}, vec2{x_min, y_max}, vec3{0,0,1}});
				vertices.push_back(vertex{vec3{(x+1)*c_width, 0, -(y+1)*c_height}, vec2{x_max, y_max}, vec3{0,0,1}});
				vertices.push_back(vertex{vec3{(x+1)*c_width, c_length, -(y+1)*c_height}, vec2{x_max, y_min}, vec3{0,0,1}});
				vertices.push_back(vertex{vec3{x*c_width, c_length, -(y+1)*c_height}, vec2{x_min, y_min}, vec3{0,0,1}});
				indices.insert(indices.end(), {size, size+1, size+2, size+2, size+3, size});

				_phys_walls.emplace_back(phys_wall_shape, 0, btVector3(x+0.5, 0.5, -(y+1)*c_height), btQuaternion{btVector3{0,1,0}, radians(180.0f)});
			}

			if (data.at(x, y-1) == 0)  // back wall
			{
				size = vertices.size();
				vertices.push_back(vertex{vec3{x*c_width, 0, -y*c_height}, vec2{x_min, y_max}, vec3{0,0,-1}});
				vertices.push_back(vertex{vec3{(x+1)*c_width, 0, -y*c_height}, vec2{x_max, y_max}, vec3{0,0,-1}});
				vertices.push_back(vertex{vec3{(x+1)*c_width, c_length, -y*c_height}, vec2{x_max, y_min}, vec3{0,0,-1}});
				vertices.push_back(vertex{vec3{x*c_width, c_length, -y*c_height}, vec2{x_min, y_min}, vec3{0,0,-1}});
				indices.insert(indices.end(), {size, size+3, size+2, size+2, size+1, size});

				_phys_walls.emplace_back(phys_wall_shape, 0, btVector3(x+0.5, 0.5, -y*c_height));
			}
		}  // x
	}  // y

	_mesh = mesh_from_vertices(vertices, indices);
}
