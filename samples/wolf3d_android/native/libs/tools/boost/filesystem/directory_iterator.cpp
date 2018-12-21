#include "directory_iterator.hpp"
#include <algorithm>

using std::swap;
using std::shared_ptr;

directory_iterator::directory_iterator() noexcept
	: _dir{nullptr}, _entry{nullptr}
{}

directory_iterator::directory_iterator(char const * dirpath)
{
	_dir = shared_ptr<DIR>{opendir(dirpath), closedir};
	if (_dir)
		next_entry();
	else
		_entry = nullptr;
}

directory_iterator::~directory_iterator()
{}

directory_iterator & directory_iterator::operator++()
{
	next_entry();
	return *this;
}

directory_entry directory_iterator::operator*()
{
	return _entry;
}

void directory_iterator::next_entry()
{
	_entry = readdir(_dir.get());
}

bool directory_iterator::operator==(directory_iterator const & rhs) const
{
	return _entry == rhs._entry;
}
