#include "operations.hpp"

// posix
#include <sys/stat.h>

namespace boost { namespace filesystem {

bool exists(path const & p)
{
	struct stat buf;
	return stat(p.c_str(), &buf) == 0;
}

uintmax_t file_size(path const & p)
{
	struct stat buf;
	if (stat(p.c_str(), &buf) == 0)
		return buf.st_size;
	else
		return -1;
}

}}  // boost::filesystem
