#include "logger.hpp"
#include <iostream>

ptr<logger> logger::DEBUG_LOGGER(nullptr);
ptr<logger> logger::INFO_LOGGER(new logger("INFO"));
ptr<logger> logger::WARNING_LOGGER(new logger("WARNING"));
ptr<logger> logger::ERROR_LOGGER(new logger("ERROR"));

void logger::log(std::string const & msg)
{
	std::cerr << _type << msg << std::endl;
}

void logger::log(std::string const & topic, std::string const & msg)
{
	std::cerr << _type << " [" << topic << "] " << msg << std::endl;
}

void logger::flush()
{
	std::cerr.flush();
}
