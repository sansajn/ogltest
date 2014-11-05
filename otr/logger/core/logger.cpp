#include "core/logger.hpp"
#include "core/cerr_logger.hpp"

ptr<logger> logger::DEBUG_LOGGER(nullptr);
ptr<logger> logger::INFO_LOGGER = make_ptr<cerr_logger>("INFO");
ptr<logger> logger::WARNING_LOGGER = make_ptr<cerr_logger>("WARNING");
ptr<logger> logger::ERROR_LOGGER = make_ptr<cerr_logger>("ERROR");


void debug_log(std::string const & msg)
{
	debug_log(std::string(), msg);
}

void debug_log(std::string const & topic, std::string const & msg)
{
	if (logger::DEBUG_LOGGER)
		logger::DEBUG_LOGGER->log(topic, msg);
}

void error_log(std::string const & msg)
{
	error_log(std::string(""), msg);
}

void error_log(std::string const & topic, std::string const & msg)
{
	if (logger::ERROR_LOGGER)
		logger::ERROR_LOGGER->log(topic, msg);
}
