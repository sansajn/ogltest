#include "html_logger.hpp"
#include <exception>

html_logger::html_logger(std::string const & type, std::string const & log_name)
	: logger(type)
{
	_out.open(log_name.c_str(), std::ios::out);
	if (!_out.is_open())
		throw std::exception();  // TODO: specify

	// html header
	_out << "<html><table>\n";
}

html_logger::~html_logger()
{
	_out << "</table></html>\n";
	_out.close();
}

void html_logger::log(std::string const & msg)
{
	_out << "<tr><td>" << _type << " " << msg << "</td></tr>\n";
}

void html_logger::log(std::string const & topic, std::string const & msg)
{
	_out << "<tr><td>" << _type << " [" << topic << "] " << msg << "</td></tr>\n";
}
