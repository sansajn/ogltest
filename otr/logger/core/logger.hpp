#pragma once
#include <string>
#include "core/ptr.hpp"


/*! \ingroup core */
class logger
{
public:
	static ptr<logger> DEBUG_LOGGER;  // TODO: len ak je debug rezim
	static ptr<logger> INFO_LOGGER;
	static ptr<logger> WARNING_LOGGER;
	static ptr<logger> ERROR_LOGGER;

	logger(std::string const & type) : _type(type) {}

	virtual ~logger() {}
	virtual void log(std::string const & msg);
	virtual void log(std::string const & topic, std::string const & msg);
	virtual void flush();

	// TODO: implement stream operator<< 

protected:
	std::string const _type;
	// TODO: synchronization support
};

inline void dlog(std::string const & s)
{
	if (logger::DEBUG_LOGGER)
		logger::DEBUG_LOGGER->log(s);
}

inline void dlog(std::string const & topic, std::string const & s)
{
	if (logger::DEBUG_LOGGER)
		logger::DEBUG_LOGGER->log(topic, s);
}
