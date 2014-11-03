#include "core/html_logger.hpp"

int main(int argc, char * argv[])
{
	logger::DEBUG_LOGGER = ptr<html_logger>(new html_logger("DEBUG", "log.html"));
	dlog("Hello");
	dlog("name", "Peter!");
	return 0;
}
