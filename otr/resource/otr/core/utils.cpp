#include "utils.hpp"
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <boost/format.hpp>

using namespace std;

std::string read_file(char const * fname)
{
	ifstream in(fname);
	assert(in.is_open() && "can't open file");
	if (!in.is_open())
		throw runtime_error(boost::str(boost::format("can't open '%1%' file") % fname));

	stringstream ss;
	ss << in.rdbuf();
	in.close();
	return ss.str();
}

std::string read_file(std::string const & fname)
{
	return read_file(fname.c_str());
}
