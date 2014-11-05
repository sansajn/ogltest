#include <iostream>
#include "core/html_logger.hpp"

using namespace std;

int main(int argc, char * argv[])
{
	logger::DEBUG_LOGGER = ptr<html_logger>(new html_logger("DEBUG", "log.html"));
	debug_log("MAIN", "Peter!");

	string resname = "camera";
	dlog("ERROR") << "resource '" << resname << "' not found";

	elog("MAIN") << "very bad thing happened ...";

	cout << "done!\n";
	return 0;
}
