// texturovany model
#include <map>
#include <vector>
#include <memory>
#include <string>
#include <utility>
#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <boost/filesystem.hpp>
#include "window.hpp"
#include "mesh.hpp"
#include "program.hpp"
#include "camera.hpp"
#include "controllers.hpp"
#include "texture.hpp"

using std::map;
using std::vector;
using std::string;
using std::shared_ptr;
using std::make_pair;
using std::runtime_error;
using glm::mat4;
using glm::mat3;
using glm::vec3;
using glm::translate;
using glm::scale;
using glm::normalize;
using glm::inverseTranspose;
using glm::radians;
using glm::rotate;
using glm::mat3_cast;
using gl::mesh;
using gl::attribute;
using gl::model;
using gl::camera;
using gl::free_look;
using gl::free_move;
using ui::glut_pool_window;

namespace fs = boost::filesystem;

string model_path = "assets/blaster/view.md5mesh";
string shaded_shader_path = "shaders/parallax.glsl";
string axis_shader_path = "shaders/colored.glsl";
string light_shader_path = "shaders/solid.glsl";

static mesh extract_mesh(aiMesh const & m);

class texture_manager
{
public:
	// get()
	// find()
	shared_ptr<texture2d> find(string const & key);  // texture_name
	void root_path(string const & path) {_root_path = path;}
	bool exists(string const & texture_name) const;

private:
	string get_texture_filename(string const & key) const;

	string _root_path;
	map<string, shared_ptr<texture2d>> _textures;
};

bool texture_manager::exists(string const & texture_name) const
{
	return fs::exists(fs::path{get_texture_filename(_root_path + texture_name)});
}

texture_manager texman;  //  global texture manager

class textured_model
{
public:
	enum texture_type
	{
		none_texture = 0,
		diffuse_texture = 1,
		normal_texture = 2,
		height_texture = 4,
		specular_texture = 8,
		glow_texture = 16
	};

	textured_model() {}
	textured_model(textured_model && other);
	void render(shader::program & prog);
	void append_mesh(std::shared_ptr<mesh> m, std::string const & texture_id, int texture_compound = diffuse_texture);
	void operator=(textured_model && other);

	textured_model(textured_model const &) = delete;
	void operator=(textured_model const &) = delete;

private:
	std::vector<std::string> _texture_ids;
	std::vector<int> _texture_compound;  // texture_type
	std::vector<std::shared_ptr<mesh>> _meshes;
};

static textured_model textured_model_from_file(string const & fname);


void textured_model::render(shader::program & prog)
{
	for (int i = 0; i < _meshes.size(); ++i)
	{
		string const & texid = _texture_ids[i];

		if (_texture_compound[i] & diffuse_texture)
		{
			shared_ptr<texture2d> diff_tex = texman.find(texid + "_d.tga");
			assert(diff_tex && "invalid texture pointer");
			diff_tex->bind(0);
			prog.uniform_variable("diff_tex", 0);
		}

		if (_texture_compound[i] & height_texture)
		{
			shared_ptr<texture2d> height_tex = texman.find(texid + "_h.tga");
			assert(height_tex && "invalid texture pointer");
			height_tex->bind(2);
			prog.uniform_variable("height_tex", 2);
		}

		_meshes[i]->render();
	}
}

void textured_model::append_mesh(std::shared_ptr<mesh> m, std::string const & texture_id, int texture_compound)
{
	_meshes.push_back(m);
	_texture_ids.push_back(texture_id);
	_texture_compound.push_back(texture_compound);
}

textured_model::textured_model(textured_model && other)
	: _texture_ids{move(other._texture_ids)}
	, _texture_compound{move(other._texture_compound)}
	, _meshes{move(other._meshes)}
{}

void textured_model::operator=(textured_model && other)
{
	swap(_texture_ids, other._texture_ids);
	swap(_texture_compound, other._texture_compound);
	swap(_meshes, other._meshes);
}

bool diffuse_exists(char const * path)
{
	return texman.exists(string{path} + "_d.tga");
}

bool local_normal_exists(char const * path)
{
	return texman.exists(string{path} + "_local.tga");
}

bool height_exists(char const * path)
{
	return texman.exists(string{path} + "_h.tga");
}

bool specular_exists(char const * path)
{
	return texman.exists(string{path} + "_s.tga");
}

bool glow_exists(char const * path)
{
	return texman.exists(string{path} + "_glow.tga");
}

textured_model textured_model_from_file(string const & fname)
{
	Assimp::Importer importer;
	aiScene const * scene = importer.ReadFile(fname,
		aiProcess_Triangulate|aiProcess_GenSmoothNormals|aiProcess_CalcTangentSpace|aiProcess_JoinIdenticalVertices);

	if (!scene)
		throw runtime_error{string{"assimp: "} + string{importer.GetErrorString()}};

	assert(scene->mNumMaterials == scene->mNumMeshes && "ocakavam texturu pre kazdu mriezku");

	textured_model mdl;
	for (int i = 0; i < scene->mNumMeshes; ++i)
	{
		aiString texture_id;
		scene->mMaterials[i]->Get(AI_MATKEY_NAME, texture_id);

		int type = textured_model::none_texture;
		if (diffuse_exists(texture_id.C_Str()))
			type |= textured_model::diffuse_texture;
		if (local_normal_exists(texture_id.C_Str()))
			type |= textured_model::normal_texture;
		if (height_exists(texture_id.C_Str()))
			type |= textured_model::height_texture;
		if (specular_exists(texture_id.C_Str()))
			type |= textured_model::specular_texture;
		if (glow_exists(texture_id.C_Str()))
			type |= textured_model::glow_texture;

		mdl.append_mesh(shared_ptr<mesh>{new mesh{extract_mesh(*scene->mMeshes[i])}}, string{texture_id.C_Str()}, type);
	}

	return mdl;
}

mesh extract_mesh(aiMesh const & m)
{
	// vertices
	vector<float> vbuf;
	unsigned vbuf_size = m.mNumVertices * (3+2+3+3);  // position, uv, normal, tangent
	vbuf.resize(vbuf_size);

	float * vptr = vbuf.data();
	for (int i = 0; i < m.mNumVertices; ++i)
	{
		aiVector3D & v = m.mVertices[i];
		*vptr++ = v.x;
		*vptr++ = v.y;
		*vptr++ = v.z;

		if (m.mTextureCoords[0])
		{
			aiVector3D & uv = m.mTextureCoords[0][i];
			*vptr++ = uv.x;
			*vptr++ = uv.y;
		}
		else  // texture uv not available
		{
			*vptr++ = 0.0f;
			*vptr++ = 0.0f;
		}

		aiVector3D & n = m.mNormals[i];
		*vptr++ = n.x;
		*vptr++ = n.y;
		*vptr++ = n.z;

		if (m.mTangents)
		{
			aiVector3D & t = m.mTangents[i];
			*vptr++ = t.x;
			*vptr++ = t.y;
			*vptr++ = t.z;
		}
		else  // tangents not available
		{
			*vptr++ = 0.0f;
			*vptr++ = 0.0f;
			*vptr++ = 0.0f;
		}
	}  // for (n

	// indices
	vector<unsigned> ibuf;
	unsigned ibuf_size = m.mNumFaces*3;  // predpoklada triangulaciu mriezky
	ibuf.resize(ibuf_size);

	unsigned * iptr = ibuf.data();
	for (int n = 0; n < m.mNumFaces; ++n)
	{
		aiFace & f = m.mFaces[n];
		*iptr++ = f.mIndices[0];
		*iptr++ = f.mIndices[1];
		*iptr++ = f.mIndices[2];
	}

	mesh result(vbuf.data(), vbuf.size()*sizeof(float), ibuf.data(), ibuf.size());
	unsigned stride = (3+2+3+3)*sizeof(GL_FLOAT);
	result.append_attribute(attribute{0, 3, GL_FLOAT, stride});  // position
	result.append_attribute(attribute{1, 2, GL_FLOAT, stride, 3*sizeof(GL_FLOAT)});  // texcoord
	result.append_attribute(attribute{2, 3, GL_FLOAT, stride, (3+2)*sizeof(GL_FLOAT)});  // normal
	result.append_attribute(attribute{3, 3, GL_FLOAT, stride, (3+2+3)*sizeof(GL_FLOAT)});  // tangent

	return result;
}

class scene_window : public glut_pool_window
{
public:
	using base = glut_pool_window;

	scene_window();
	void display() override;
	void input(float dt) override;

private:
	textured_model _textured;
	shader::program _prog;
	camera _cam;
	free_look<scene_window> _look;
	free_move<scene_window> _move;

	// debug stuff
	mesh _axis;
	mesh _light;
	shader::program _axis_prog;
	shader::program _light_prog;
};

shared_ptr<texture2d> texture_manager::find(string const & key)
{
	auto it = _textures.find(key);
	if (it != _textures.end())
		return it->second;

	auto result = _textures.insert(
		make_pair(key, shared_ptr<texture2d>{new texture2d{get_texture_filename(key)}}));

	return result.first->second;
}

string texture_manager::get_texture_filename(string const & key) const
{
	string::size_type pos = key.rfind('/');
	if (pos != string::npos && pos < key.size()-1)
		return _root_path + "/" + key.substr(pos+1);
	else
		return string{};
}

scene_window::scene_window() : _look{_cam, *this}, _move{_cam, *this}
{
	_textured = textured_model_from_file(model_path);
	_prog.from_file(shaded_shader_path);
	_cam = camera{radians(70.0f), aspect_ratio(), 0.01, 1000};
	_cam.look_at(vec3{1,0,0});
	_axis = gl::make_axis();
	_light = gl::make_sphere();
	_axis_prog.from_file(axis_shader_path);
	_light_prog.from_file(light_shader_path);
}

void scene_window::display()
{
	mat4 M = mat4{1};
	M = rotate(M, radians(-90.0f), vec3{1, 0, 0});
	mat4 world_to_camera = _cam.view();
	mat4 local_to_screen = _cam.view_projection() * M;
	mat3 normal_to_camera = mat3{inverseTranspose(world_to_camera*M)};

	vec3 light_pos = vec3{5,5,-5};

	_prog.use();
	_prog.uniform_variable("local_to_world", M);
	_prog.uniform_variable("world_to_camera", world_to_camera);
	_prog.uniform_variable("local_to_screen", local_to_screen);
	_prog.uniform_variable("normal_to_camera", normal_to_camera);
	_prog.uniform_variable("light.direction", normalize(light_pos));

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	_textured.render(_prog);

	// light
	mat4 M_light = translate(mat4{1}, light_pos);
	M_light = scale(M_light, vec3{0.1, 0.1, 0.1});
	_light_prog.use();
	_light_prog.uniform_variable("color", vec3{1,1,0});  // yellow
	_light_prog.uniform_variable("local_to_screen", _cam.view_projection() * M_light);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	_light.render();

	// axis
	_axis_prog.use();
	_axis_prog.uniform_variable("local_to_screen", _cam.view_projection());

	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	_axis.render();

	base::display();
}

void scene_window::input(float dt)
{
	_look.input(dt);
	_move.input(dt);
	base::input(dt);
}

int main(int argc, char * argv[])
{
	texman.root_path("assets/blaster");

	scene_window w;
	w.start();
	return 0;
}
