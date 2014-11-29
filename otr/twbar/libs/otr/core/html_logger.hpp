#pragma once
#include <map>
#include <mutex>
#include <fstream>
#include "core/logger.hpp"


/*! \ingroup core */
class html_logger : public logger
{
public:
	html_logger(std::string const & type, std::string const & log_name);
	~html_logger();

	void log(std::string const & topic, std::string const & msg) override;
	void flush() override;

private:
	ptr<std::ofstream> _out;
	std::mutex _m;
	static std::map<std::string, ptr<std::ofstream>> _logs;
};
