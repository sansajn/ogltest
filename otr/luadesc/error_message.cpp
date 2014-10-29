#include "error_message.hpp"
#include <cstdio>

namespace lua {

void lmessage(char const * msg)
{
	fprintf(stderr, "%s\n", msg);
	fflush(stderr);
}

}  // lua
