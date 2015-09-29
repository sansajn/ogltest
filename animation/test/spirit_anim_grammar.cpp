// parsule md5anim suboor pomocou spiritu
#include <tuple>
#include <string>
#include <fstream>

//#define BOOST_SPIRIT_DEBUG
#define BOOST_SPIRIT_USE_PHOENIX_V3
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>

std::string anim_file = "bob_lamp.md5anim";

using pair_type = std::tuple<float, float>;
using triplet_type = std::tuple<float, float, float>;

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
};

int main(int argc, char * argv[])
{
	if (argc > 1)
		anim_file = argv[1];

	std::ifstream in{anim_file};
	if (!in.is_open())
		throw std::logic_error{std::string{"unable to open file '"} + anim_file + "'"};

	in.unsetf(std::ios::skipws);

	using iterator_type = boost::spirit::istream_iterator;
	iterator_type it{in}, end;

	md5_anim anim;

	md5_anim_grammar<iterator_type> g;
	bool match = qi::phrase_parse(it, end, g, ascii::blank, anim);
	if (!match || it != end)
		throw std::logic_error{std::string{"unable to parse an expression"}};

	std::cout << "done!\n";

	return 0;
}
