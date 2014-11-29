#include "html_logger.hpp"
#include <exception>

static void write_header(std::ofstream & out);
static void write_footer(std::ofstream & out);

std::map<std::string, ptr<std::ofstream>> html_logger::_logs;

html_logger::html_logger(std::string const & type, std::string const & log_name)
	: logger(type)
{
	std::lock_guard<std::mutex> lg(_m);
	auto it = _logs.find(log_name);
	if (it != _logs.end())
		_out = it->second;
	else
	{
		_out = ptr<std::ofstream>(new std::ofstream(log_name));
		if (_out->is_open())
		{
			write_header(*_out);
			_logs[log_name] = _out;
		}
		else
			throw std::exception();  // TODO: specify (can't open log file)
	}
}

html_logger::~html_logger()
{	
	std::lock_guard<std::mutex> lg(_m);
	if (_out.use_count() == 2)
	{
		write_footer(*_out);
		_out->close();
		_out.reset();
	}
}

void html_logger::log(std::string const & topic, std::string const & msg)
{
	std::lock_guard<std::mutex> lg(_m);
	if (topic.empty())
		*_out << "<tr><td>" << _type << " " << msg << "</td></tr>\n";
	else
		*_out << "<tr><td>" << _type << " [" << topic << "] " << msg << "</td></tr>\n";
}

void html_logger::flush()
{
	std::lock_guard<std::mutex> lg(_m);
	_out->flush();
}

void write_header(std::ofstream & out)
{
	out << "<html><table>\n";
}

void write_footer(std::ofstream & out)
{
	out << "</table></html>\n";
}
