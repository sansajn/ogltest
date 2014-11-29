#pragma once
#include <mutex>
#include <string>
#include "core/logger.hpp"

class cerr_logger : public logger
{
public:
	cerr_logger(std::string const & type) : logger(type) {}
	void log(std::string const & topic, std::string const & msg) override;

private:
	static std::mutex _m;
};
