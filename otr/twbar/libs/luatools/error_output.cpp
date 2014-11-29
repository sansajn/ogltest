#include "error_output.hpp"
#include <cstdio>

namespace lua {

void stderr_output(char const * msg)
{
	fprintf(stderr, "%s\n", msg);
	fflush(stderr);
}

}  // lua
