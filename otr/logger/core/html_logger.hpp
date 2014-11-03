#pragma once
#include <fstream>
#include "core/logger.hpp"


/*! \ingroup core */
class html_logger : public logger
{
public:
	html_logger(std::string const & type, std::string const & log_name);
	~html_logger();

	void log(std::string const & msg) override;
	void log(std::string const & topic, std::string const & msg) override;
	void flush() override {_out.flush();}

private:
	std::ofstream _out;
};
