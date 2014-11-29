#include "cerr_logger.hpp"
#include <iostream>

std::mutex cerr_logger::_m;

void cerr_logger::log(std::string const & topic, std::string const & msg)
{
	std::lock_guard<std::mutex> lk(_m);
	if (topic.empty())
		std::cerr << _type << " " << msg << std::endl;
	else
		std::cerr << _type << " [" << topic << "] " << msg << std::endl;
}
