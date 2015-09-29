// pokus o parsovanie md5mesh suboru
#include <tuple>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

//#define BOOST_SPIRIT_DEBUG
#define BOOST_SPIRIT_USE_PHOENIX_V3
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>

std::string md5_file = "bob_lamp";

using pair_type = std::tuple<float, float>;
using triplet_type = std::tuple<float, float, float>;

struct joint_record
{
	std::string name;
	int parent;
	triplet_type position;
	triplet_type orientation;
};

struct vertex_record
{
	int index;
	pair_type uv;  //!< texture coordinate mapping
	int start_weight;
	int weight_count;
};

struct triangle_record
{
	int index;
	int v0, v1, v2;  //!< vertex indices
};

struct weight_record
{
	int index;
	int joint;
	float bias;
	triplet_type position;
};

struct mesh  // submesh, part
{
	std::string shader;
	unsigned vertex_count;
	std::vector<vertex_record> vertices;
	unsigned triangle_count;
	std::vector<triangle_record> triangles;
	unsigned weight_count;
	std::vector<weight_record> weights;
};

struct md5_mesh  // scene, model
{
	unsigned version;
	std::string command;
	unsigned joint_count;
	unsigned mesh_count;
	std::vector<joint_record> joints;
	std::vector<mesh> meshes;
};

struct hierarchy_record
{
	std::string name;
	int parent;
	int flags;
	int start_index;
};

struct bound_record
{
	triplet_type min_corner;
	triplet_type max_corner;
};

struct baseframe_record
{
	triplet_type position;
	triplet_type orientation;
};

struct frame
{
	std::vector<float> data;
};

struct md5_anim
{
	int version;
	std::string command;
	unsigned frame_count;
	unsigned joint_count;
	unsigned frame_rate;
	unsigned animated_component_count;
	std::vector<hierarchy_record> hierarchy;
	std::vector<bound_record> bounds;
	std::vector<baseframe_record> baseframe;
	std::vector<frame> frames;
};

BOOST_FUSION_ADAPT_STRUCT(
	md5_mesh,
	(unsigned, version)
	(std::string, command)
	(unsigned, joint_count)
	(unsigned, mesh_count)
	(std::vector<joint_record>, joints)
	(std::vector<mesh>, meshes)
);

BOOST_FUSION_ADAPT_STRUCT(
	mesh,
	(std::string, shader)
	(unsigned, vertex_count)
	(std::vector<vertex_record>, vertices)
	(unsigned, triangle_count)
	(std::vector<triangle_record>, triangles)
	(unsigned, weight_count)
	(std::vector<weight_record>, weights)
);

BOOST_FUSION_ADAPT_STRUCT(
	joint_record,
	(std::string, name)
	(int, parent)
	(triplet_type, position)
	(triplet_type, orientation)
);

BOOST_FUSION_ADAPT_STRUCT(
	vertex_record,
	(int, index)
	(pair_type, uv)
	(int, start_weight)
	(int, weight_count)
);

BOOST_FUSION_ADAPT_STRUCT(
	triangle_record,
	(int, index)
	(int, v0)
	(int, v1)
	(int, v2)
);

BOOST_FUSION_ADAPT_STRUCT(
	weight_record,
	(int, index)
	(int, joint)
	(float, bias)
	(triplet_type, position)
);

BOOST_FUSION_ADAPT_STRUCT(
	md5_anim,
	(int, version)
	(std::string, command)
	(unsigned, frame_count)
	(unsigned, joint_count)
	(unsigned, frame_rate)
	(unsigned, animated_component_count)
	(std::vector<hierarchy_record>, hierarchy)
	(std::vector<bound_record>, bounds)
	(std::vector<baseframe_record>, baseframe)
	(std::vector<frame>, frames)
);

BOOST_FUSION_ADAPT_STRUCT(
	hierarchy_record,
	(std::string, name)
	(int, parent)
	(int, flags)
	(int, start_index)
);

BOOST_FUSION_ADAPT_STRUCT(
	bound_record,
	(triplet_type, min_corner)
	(triplet_type, max_corner)
);

BOOST_FUSION_ADAPT_STRUCT(
	baseframe_record,
	(triplet_type, position)
	(triplet_type, orientation)
);

BOOST_FUSION_ADAPT_STRUCT(
	frame,
	(std::vector<float>, data)
);


namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

template <typename Iterator>
struct md5_mesh_grammar : qi::grammar<Iterator, md5_mesh(), ascii::blank_type>
{
	md5_mesh_grammar() : md5_mesh_grammar::base_type{md5}
	{
		using qi::lexeme;
		using qi::char_;
		using qi::int_;
		using qi::float_;
		using qi::lit;  // literal

		md5 =
				*comment
			>> "MD5Version" >> int_ >> end
			>> "commandline" >> qstring >> end
			>> "numJoints" >> int_ >> end
			>> "numMeshes" >> int_ >> end
			>> joints
			>> +mesh_;

		joints =
				lit("joints") >> '{' >> end
			>> *(qstring >> int_ >> triplet >> triplet >> end)
			>> '}' >> end;

		mesh_ =
				lit("mesh") >> '{' >> end
			>> "shader" >> qstring >> end
			>> "numverts" >> int_ >> end
			>> *vert
			>> "numtris" >> int_ >> end
			>> *tri
			>> "numweights" >> int_ >> end
			>> *weight
			>> '}' >> end;

		vert = "vert" >> int_ >> pair >> int_ >> int_ >> end;
		tri = "tri" >> int_ >> int_ >> int_ >> int_ >> end;
		weight = "weight" >> int_ >> int_ >> float_ >> triplet >> end;
		triplet = '(' >> float_ >> float_ >> float_ >> ')';
		pair = '(' >> float_ >> float_ >> ')';
		qstring = lexeme['"' >> *(char_ - '"') >> '"'];
		comment = "//" >> *(char_ - eol) >> eol;
		end = +comment|eol;
		eol = +qi::eol;

		BOOST_SPIRIT_DEBUG_NODE(md5);
		BOOST_SPIRIT_DEBUG_NODE(joints);
		BOOST_SPIRIT_DEBUG_NODE(mesh_);
		BOOST_SPIRIT_DEBUG_NODE(vert);
		BOOST_SPIRIT_DEBUG_NODE(tri);
		BOOST_SPIRIT_DEBUG_NODE(weight);
		BOOST_SPIRIT_DEBUG_NODE(triplet);
		BOOST_SPIRIT_DEBUG_NODE(pair);
		BOOST_SPIRIT_DEBUG_NODE(qstring);
		BOOST_SPIRIT_DEBUG_NODE(comment);
		BOOST_SPIRIT_DEBUG_NODE(end);
		BOOST_SPIRIT_DEBUG_NODE(eol);
	}

	qi::rule<Iterator, md5_mesh(), ascii::blank_type> md5;
	qi::rule<Iterator, std::vector<joint_record>(), ascii::blank_type> joints;
	qi::rule<Iterator, mesh(), ascii::blank_type> mesh_;
	qi::rule<Iterator, vertex_record(), ascii::blank_type> vert;
	qi::rule<Iterator, triangle_record(), ascii::blank_type> tri;
	qi::rule<Iterator, weight_record(), ascii::blank_type> weight;
	qi::rule<Iterator, triplet_type(), ascii::blank_type> triplet;
	qi::rule<Iterator, pair_type(), ascii::blank_type> pair;
	qi::rule<Iterator, std::string(), ascii::blank_type> qstring;
	qi::rule<Iterator, void(), ascii::blank_type> comment;
	qi::rule<Iterator, void(), ascii::blank_type> end;
	qi::rule<Iterator, void(), ascii::blank_type> eol;
};  // md5_mesh_grammar

template <typename Iterator>
struct md5_anim_grammar : qi::grammar<Iterator, md5_anim(), ascii::blank_type>
{
	md5_anim_grammar() : md5_anim_grammar::base_type{md5}
	{
		using qi::char_;
		using qi::int_;
		using qi::float_;
		using qi::lit;  // literal
		using qi::lexeme;

		md5 =
				lit("MD5Version") >> int_ >> endl
			>> "commandline" >> qstring >> endl
			>> "numFrames" >> int_ >> endl
			>> "numJoints" >> int_ >> endl
			>> "frameRate" >> int_ >> endl
			>> "numAnimatedComponents" >> int_ >> endl
			>> hierarchy
			>> bounds
			>> baseframe
			>> *frame_;

		hierarchy =
				lit("hierarchy") >> '{' >> endl
			>> *(qstring >> int_ >> int_ >> int_ >> endl)
			>> '}' >> endl;

		bounds =
				lit("bounds") >> '{' >> endl
			>> *(triplet >> triplet >> endl)
			>> '}' >> endl;

		baseframe =
				lit("baseframe") >> '{' >> endl
			>> *(triplet >> triplet >> endl)
			>> '}' >> endl;

		frame_ =
				lit("frame") >> int_ >> '{' >> endl
			>> *(float_|endl)
			>> '}' >> endl;

		triplet = '(' >> float_ >> float_ >> float_ >> ')';
		pair = '(' >> float_ >> float_ >> ')';
		qstring = lexeme['"' >> *(char_ - '"') >> '"'];
		comment = "//" >> *(char_ - qi::eol) >> eol;
		endl = +comment|eol;
		eol = +qi::eol;

		BOOST_SPIRIT_DEBUG_NODE(md5);
		BOOST_SPIRIT_DEBUG_NODE(hierarchy);
		BOOST_SPIRIT_DEBUG_NODE(bounds);
		BOOST_SPIRIT_DEBUG_NODE(baseframe);
		BOOST_SPIRIT_DEBUG_NODE(frame_);
		BOOST_SPIRIT_DEBUG_NODE(triplet);
		BOOST_SPIRIT_DEBUG_NODE(pair);
		BOOST_SPIRIT_DEBUG_NODE(qstring);
		BOOST_SPIRIT_DEBUG_NODE(comment);
		BOOST_SPIRIT_DEBUG_NODE(endl);
		BOOST_SPIRIT_DEBUG_NODE(eol);
	}

	qi::rule<Iterator, md5_anim(), ascii::blank_type> md5;
	qi::rule<Iterator, std::vector<hierarchy_record>(), ascii::blank_type> hierarchy;
	qi::rule<Iterator, std::vector<bound_record>(), ascii::blank_type> bounds;
	qi::rule<Iterator, std::vector<baseframe_record>(), ascii::blank_type> baseframe;
	qi::rule<Iterator, frame(), ascii::blank_type> frame_;
	qi::rule<Iterator, triplet_type(), ascii::blank_type> triplet;
	qi::rule<Iterator, pair_type(), ascii::blank_type> pair;
	qi::rule<Iterator, std::string(), ascii::blank_type> qstring;
	qi::rule<Iterator, void(), ascii::blank_type> comment;
	qi::rule<Iterator, void(), ascii::blank_type> endl;
	qi::rule<Iterator, void(), ascii::blank_type> eol;
};  // md5_anim_grammar


int main(int argc, char * argv[])
{
	if (argc > 1)
		md5_file = argv[1];

	std::ifstream in{md5_file + ".md5mesh"};
	if (!in.is_open())
		throw std::logic_error{std::string{"unable to open file '"} + md5_file + ".md5mesh'"};

	in.unsetf(std::ios::skipws);

	using iterator_type = boost::spirit::istream_iterator;
	iterator_type it{in}, end;

	md5_mesh model;
	md5_mesh_grammar<iterator_type> mesh_g;
	bool match = qi::phrase_parse(it, end, mesh_g, ascii::blank, model);
	if (!match || it != end)
		throw std::logic_error{std::string{"unable to parse a mesh file"}};

	in.close();

	in.open(md5_file + ".md5anim");
	if (!in.is_open())
		throw std::logic_error{std::string{"unable to open file '"} + md5_file + ".md5anim'"};

	in.unsetf(std::ios::skipws);

	it = iterator_type{in};

	md5_anim anim;
	md5_anim_grammar<iterator_type> anim_g;
	match = qi::phrase_parse(it, end, anim_g, ascii::blank, anim);
	if (!match || it != end)
		throw std::logic_error{std::string{"unable to parse an animation file"}};

	std::cout << "done!\n";

	return 0;
}

