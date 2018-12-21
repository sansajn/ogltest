#pragma once
#include <memory>
#include <dirent.h>  // posix api

using directory_entry = dirent *;

/*! Implementacia directory iterator-a.
Umoznuje iterovat obsah adresara (directory_iterator implementuje input iterator koncept). */
class directory_iterator
{
public:
	directory_iterator() noexcept;
	explicit directory_iterator(char const * dirpath);
	~directory_iterator();
	directory_iterator & operator++();  // pre-increment operator
	directory_entry operator*();
	bool operator==(directory_iterator const & rhs) const;
	bool operator!=(directory_iterator const & rhs) const {return !(*this == rhs);}

private:
	void next_entry();

	std::shared_ptr<DIR> _dir;
	dirent * _entry;
};

inline directory_iterator const & begin(directory_iterator const & iter) {return iter;}
inline directory_iterator end(directory_iterator const &) {return directory_iterator{};}
