#include "path.hpp"
#include <utility>

namespace boost { namespace filesystem {

path & path::operator=(path const & p)
{
	_pathname = p._pathname;
	return *this;
}

path & path::operator/=(path const & p)
{
	// TODO: optimalizuj reserve p._pathname.size() + 1
	if (_pathname.back() != '/')
		_pathname += "/";
	_pathname += p._pathname;
	return *this;
}

path & path::operator+=(path const & p)
{
	_pathname += p._pathname;
	return *this;
}

void path::clear()
{
	_pathname.clear();
}

path & path::remove_filename()
{
	size_t last_slash_pos = _pathname.rfind('/');
	if (last_slash_pos == std::string::npos)
		_pathname = "";
	else
		_pathname.assign(_pathname.begin(), _pathname.begin() + last_slash_pos);
	return *this;
}

path & path::replace_extension(path const & newext)
{
	// TODO: mozem mat relatyvnu cestu s bodkou
	size_t dot_pos = _pathname.rfind('.');
	if (dot_pos == std::string::npos)
		_pathname += "." + newext._pathname;
	else if (dot_pos == _pathname.size()-1)
		_pathname += newext._pathname;
	else
		_pathname.replace(_pathname.begin() + dot_pos + 1, _pathname.end(), newext._pathname);
	return *this;
}

void path::swap(path & rhs)
{
	std::swap(_pathname, rhs._pathname);
}

path path::filename() const
{
	if (_pathname == "/" || _pathname == ".." || _pathname == ".")  // root, relative
		return *this;

	size_t last_slash_pos = _pathname.rfind('/');
	if (last_slash_pos == _pathname.size()-1)
		return path{"."};
	else
		return path{std::string{_pathname.begin() + last_slash_pos + 1, _pathname.end()}};
}

path path::root_directory() const
{
	if (!empty() && _pathname[0] == '/')
		return path{"/"};
	else
		return path{};
}

path path::parent_path() const
{
	if (_pathname == "/" || _pathname == ".." || _pathname == ".")  // root, relative
		return path{};

	size_t last_slash_pos = _pathname.rfind('/');
	if (last_slash_pos == std::string::npos)
		return path{};
	else
		return path{_pathname.begin(), _pathname.begin() + last_slash_pos};
}

path path::extension() const
{
	path fn = filename();
	size_t dot_pos = fn._pathname.rfind('.');
	if (dot_pos == std::string::npos)
		return path{};
	else
		return path{std::string{fn._pathname.begin() + dot_pos, fn._pathname.end()}};
}

bool path::has_root_directory() const
{
	return !root_directory().empty();
}

bool path::has_filename() const
{
	return !filename().empty();
}

bool path::has_extension() const
{
	return !extension().empty();
}

}}  // boost::filesystem
