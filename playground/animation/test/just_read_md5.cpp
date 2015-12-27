// pomocou assimp-u precita md5mesh subor 
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <utility>
#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

using std::transform;
using std::string;
using std::vector;
using std::list;
using std::map;
using std::set;
using std::make_pair;
using std::system;
using std::ofstream;
using std::cout;

string const model_mesh = "../assets/bob/bob_lamp_update_export.md5mesh";

void create_graphviz_dot(aiNode * root, string const & fname);
void graph_traverse(aiNode * n, map<string, set<string>> & result);

void print_node_hierarchy(aiNode * root);
void print_node(aiNode * n, int & depth, vector<list<string>> & names);


int main(int argc, char * argv[])
{
	Assimp::Importer importer;
	aiScene const * scene = importer.ReadFile(model_mesh, 0);
	assert(scene && "unable to read a model file");

	cout << "{scene-info}\n"
		<< "animations:" << scene->mNumAnimations << "\n"
		<< "cameras:" << scene->mNumCameras << "\n"
		<< "lights:" << scene->mNumLights << "\n"
		<< "materials:" << scene->mNumMaterials << "\n"
		<< "meshes:" << scene->mNumMeshes << "\n"
		<< "textures:"  << scene->mNumTextures << "\n\n";

	// meshes
	cout << "{meshes}\n";
	for (int i = 0; i < scene->mNumMeshes; ++i)
	{
		aiMesh * mesh = scene->mMeshes[i];
		cout << "mesh" << i << ":" << mesh->mName.C_Str() << "\n"
			<< "bones:" << mesh->mNumBones << "\n"
			<< "faces:" << mesh->mNumFaces << "\n"
			<< "vertices:" << mesh->mNumVertices << "\n"
			<< "has-normals:" << mesh->HasNormals() << "\n"
			<< "color-channels:" << mesh->GetNumColorChannels() << "\n"
			<< "uv-channels:" << mesh->GetNumUVChannels() << "\n\n";
	}

	// animations
	cout << "{animations}\n";
	for (int i = 0; i < scene->mNumAnimations; ++i)
	{
		aiAnimation * anim = scene->mAnimations[i];
		cout << "name:" << anim->mName.C_Str() << "\n"
			<< "duration:" << anim->mDuration / anim->mTicksPerSecond << "s\n"
			<< "bone-channels:" << anim->mNumChannels << "\n"
			<< "mesh-channels:" << anim->mNumMeshChannels << "\n\n";
	}

	aiNode * root = scene->mRootNode;
	cout << "{nodes}\n";
	print_node_hierarchy(root);
	create_graphviz_dot(root, "out/node.gv");

	return 0;
}

string transform_vertex_name(string const & s)
{
	string result(s.size(), ' ');
	transform(s.begin(), s.end(), result.begin(), [](auto & ch){return (ch == '.') ? '_' : ch;});
	return result;
}

void create_graphviz_dot(aiNode * root, string const & fname)
{
	map<string, set<string>> g;
	graph_traverse(root, g);

	ofstream out{fname};
	if (!out.is_open())
		return;

	out << "// $ dot -Tpng graph.gv -o graph.png\n"
		<< "digraph G {\n";
	for (map<string, set<string>>::const_iterator it = g.cbegin(); it != g.cend(); ++it)
	{
		out << "\t" << transform_vertex_name(it->first);
		if (!it->second.empty())
		{
			out << " -> {";
			int count = 0;
			for (string name : it->second)
			{
				out << transform_vertex_name(name);
				if (count++ < (it->second.size() - 1))
					out << ", ";
			}
			out << "};";
		}
		out << "\n";
	}
	out << "}\n\n";

	out.close();

	string cmd = string{"dot -Tpng "} + string{fname} + " -o " + string{fname} + ".png";
	system(cmd.c_str());

	cout << "file '" << fname << "' created, see " << fname << ".png for detail info\n";
}

void graph_traverse(aiNode * n, map<string, set<string>> & result)
{
	string name = n->mName.C_Str();
	auto it = result.find(name);
	if (it == result.end())
		it = result.insert(make_pair(name, set<string>{})).first;

	for (int i = 0; i < n->mNumChildren; ++i)
	{
		aiNode * ch = n->mChildren[i];
		it->second.insert(ch->mName.C_Str());
		graph_traverse(ch, result);
	}
}

void print_node_hierarchy(aiNode * root)
{
	vector<list<string>> names;
	int depth = 0;
	print_node(root, depth, names);

	for (list<string> & names_at_depth : names)
	{
		for (string & name : names_at_depth)
			cout << name << ", ";
		cout << "\n";
	}
}

void print_node(aiNode * n, int & depth, vector<list<string>> & names)
{
	depth += 1;
	if (depth >= names.size())
		names.emplace_back();

	names[depth-1].push_back(n->mName.C_Str());

	for (int i = 0; i < n->mNumChildren; ++i)
		print_node(n->mChildren[i], depth, names);

	depth -= 1;
}
