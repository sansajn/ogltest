#pragma once
#include <string>
#include <sstream>
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

	virtual ~logger() {flush();}
	virtual void log(std::string const & topic, std::string const & msg) = 0;
	virtual void flush() {}

	class oneliner  //!< stream support (operator<<())
	{
	public:
		oneliner(ptr<logger> out, std::string const & topic = std::string()) : _out(out), _topic(topic) {}
		oneliner(oneliner && rhs) : _out(rhs._out), _topic(rhs._topic), _buf(rhs._buf) {}  // FIXME: namiesto move data len kopirujem

		~oneliner()
		{
			if (_out)
				_out->log(_topic, _buf);
		}

		template <typename T>
		oneliner & operator<<(T const & v)
		{
			if (_out)
			{
				std::ostringstream oss;
				oss << v;
				_buf += oss.str();
			}
			return *this;
		}

		oneliner(oneliner const &) = delete;
		oneliner & operator=(oneliner const &) = delete;

	private:
		ptr<logger> _out;
		std::string _topic, _buf;
	};

protected:
	std::string const _type;
};

void debug_log(std::string const & topic, std::string const & msg);
void info_log(std::string const & topic, std::string const & msg);
void warning_log(std::string const & topic, std::string const & msg);
void error_log(std::string const & topic, std::string const & msg);

inline logger::oneliner dlog(std::string const & topic) {return logger::oneliner(logger::DEBUG_LOGGER, topic);}
inline logger::oneliner ilog(std::string const & topic) {return logger::oneliner(logger::INFO_LOGGER, topic);}
inline logger::oneliner wlog(std::string const & topic) {return logger::oneliner(logger::WARNING_LOGGER, topic);}
inline logger::oneliner elog(std::string const & topic) {return logger::oneliner(logger::ERROR_LOGGER, topic);}
