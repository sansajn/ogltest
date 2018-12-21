#include "md5loader.hpp"
#include <stdexcept>
#include <fstream>
#include <boost/filesystem/operations.hpp>

using std::string;
using std::vector;
using std::logic_error;
using std::ifstream;
using glm::quat;
using geom::box3;

namespace fs = boost::filesystem;

namespace md5 {

void remove_quotes(string & str)
{
	 size_t n;
	 while ( ( n = str.find('\"') ) != string::npos ) str.erase(n,1);
}

void compute_quat_w(quat & q)
{
	 float t = 1.0f - (q.x * q.x) - (q.y * q.y) - (q.z * q.z);
	 if (t < 0.0f)
		  q.w = 0.0f;
	 else
		  q.w = -sqrt(t);
}

void animation::load(string const & fname)
{
	ifstream in{fname};
	if (!in.is_open())
		throw logic_error{"unable to open an animation file '" + fname + "'"};

	fs::path fpath = fname;
	size_t flength = fs::file_size(fpath);

	version = 0;
	frame_rate = 0;
	animated_component_count = 0;
	hierarchy.clear();
	bounds.clear();
	base_frame.clear();
	frames.clear();

	string word, junk;
	unsigned frame_count = 0;
	unsigned joint_count = 0;

	in >> word;
	while (!in.eof())
	{
		if (word == "MD5Version")
		{
			in >> version;
			assert(version == 10);
		}
		else if (word == "commandline")
			in.ignore(flength, '\n');
		else if (word == "numFrames")
		{
			in >> frame_count;
			frames.resize(frame_count);
			in.ignore(flength, '\n');
		}
		else if (word == "numJoints")
		{
			in >> joint_count;
			hierarchy.resize(joint_count);
			in.ignore(flength, '\n');
		}
		else if (word == "frameRate")
		{
			in >> frame_rate;
			in.ignore(flength, '\n');
		}
		else if (word == "numAnimatedComponents")
		{
			in >> animated_component_count;
			in.ignore(flength, '\n');
		}
		else if (word == "hierarchy")
		{
			in >> junk;  // '{'
			for (int i = 0; i < joint_count; ++i)
			{
				hierarchy_record & r = hierarchy[i];
				in >> r.name >> r.parent >> r.flags >> r.start_idx;
				remove_quotes(r.name);
				in.ignore(flength, '\n');
			}
			in >> junk;  // '}'
		}
		else if (word == "bounds")
		{
			in >> junk;  // '{'
			in.ignore(flength, '\n');
			for (int i = 0; i < frame_count; ++i)
			{
				box3 b;
				in >> junk >> b.min_corner().x >> b.min_corner().y >> b.min_corner().z >> junk
					>> junk >> b.max_corner().x >> b.max_corner().y >> b.max_corner().z >> junk;
				bounds.push_back(b);
				in.ignore(flength, '\n');
			}
			in >> junk;  // '}'
			in.ignore(flength, '\n');
		}
		else if (word == "baseframe")
		{
			in >> junk;  // '{'
			in.ignore(flength, '\n');
			for (int i = 0; i < joint_count; ++i)
			{
				base_frame_record f;
				in >> junk >> f.position.x >> f.position.y >> f.position.z >> junk
					>> junk >> f.orientation.x >> f.orientation.y >> f.orientation.z >> junk;
				in.ignore(flength, '\n');
				base_frame.push_back(f);
			}
			in >> junk;  // '}'
			in.ignore(flength, '\n');
		}
		else if (word == "frame")
		{
			vector<float> f;
			unsigned frame_idx;
			in >> frame_idx >> junk;
			in.ignore(flength, '\n');

			f.reserve(animated_component_count);
			for (int i = 0; i < animated_component_count; ++i)
			{
				float d;
				in >> d;
				f.push_back(d);
			}

			assert(frame_idx < frames.size());
			swap(frames[frame_idx], f);

			in >> junk;  // '}'
			in.ignore(flength, '\n');
		}

		in >> word;
	}  // whiel (!in.eof

	assert(hierarchy.size() == joint_count);
	assert(bounds.size() == frame_count);
	assert(base_frame.size() == joint_count);
	assert(frames.size() == frame_count);
}

void model::load(string const & fname)
{
	ifstream in{fname};
	if (!in.is_open())
		throw logic_error{"unable to open a model file '" + fname + "'"};

	size_t flength = fs::file_size(fname);
	assert(flength > 0 && "empty model file");

	joints.clear();
	meshes.clear();

	string word, junk;
	unsigned mesh_count = 0;
	unsigned joint_count = 0;

	in >> word;

	while (!in.eof())
	{
		if (word == "MD5Version")
		{
			in >> version;
			assert(version == 10 && "only version 10 (doom3, quake4) supported");
		}
		else if (word == "commandline")
			in.ignore(flength, '\n');
		else if (word == "numJoints")
		{
			in >> joint_count;
			joints.reserve(joint_count);
		}
		else if (word == "numMeshes")
		{
			in >> mesh_count;
			meshes.reserve(mesh_count);
		}
		else if (word == "joints")
		{
			joint j;
			in >> junk;  // '{'
			for (unsigned i = 0; i < joint_count; ++i)
			{
				in >> j.name >> j.parent
					>> junk >> j.position.x >> j.position.y >> j.position.z >> junk
					>> junk >> j.orientation.x >> j.orientation.y >> j.orientation.z >> junk;
				remove_quotes(j.name);
				compute_quat_w(j.orientation);

				joints.push_back(j);

				in.ignore(flength, '\n');
			}

			in >> junk;  // '}'
		}  // joints
		else if (word == "mesh")
		{
			mesh m;
			in >> junk >> word;  // ignore '{'
			while (word != "}")
			{
				if (word == "shader")
				{
					in >> m.shader;
					remove_quotes(m.shader);
					in.ignore(flength, '\n');
				}
				else if (word == "numverts")
				{
					unsigned numverts;
					in >> numverts;
					in.ignore(flength, '\n');
					for (unsigned i = 0; i < numverts; ++i)
					{
						vertex v;
						in >> junk >> junk  // ignore vert, vertIndex
							>> junk >> v.uv.x >> v.uv.y >> junk
							>> v.wid >> v.wcount;
						v.uv.y = 1 - v.uv.y;  // flip uv \note pri citani textury ju flipnem, md5 obsahuje uz flipnute uv
						in.ignore(flength, '\n');

						m.vertices.push_back(v);
					}
				}
				else if (word == "numtris")
				{
					unsigned numtris;
					in >> numtris;
					in.ignore(flength, '\n');
					for (unsigned i = 0; i < numtris; ++i)
					{
						unsigned v0, v1, v2;
						in >> junk >> junk >> v0 >> v1 >> v2;
						in.ignore(flength, '\n');

						m.indices.push_back(v2);  // reverse vertices order to be CCW
						m.indices.push_back(v1);
						m.indices.push_back(v0);
					}
				}
				else if (word == "numweights")
				{
					unsigned numweights;
					in >> numweights;
					in.ignore(flength, '\n');
					for (unsigned i = 0; i < numweights; ++i)
					{
						weight w;
						in >> junk >> junk >> w.jid >> w.influence
							>> junk >> w.position.x >> w.position.y >> w.position.z >> junk;
						in.ignore(flength, '\n');

						m.weights.push_back(w);
					}
				}
				else
					in.ignore(flength, '\n');

				in >> word;
			}

			meshes.push_back(m);
		}  // mesh

		in >> word;
	}

	if (meshes.size() != mesh_count || joints.size() != joint_count)
		throw logic_error{string{"model file '"} + fname + "' is corrupted"};
}

}  // md5
