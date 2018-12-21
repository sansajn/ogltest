#pragma once
#include <string>

namespace boost { namespace filesystem {

/*! boost::filesystem::path like implementation without boost dependencies */
class path
{
public:
	using string_type = std::string;
	using value_type = std::string::value_type;

	path() {}

	template<typename InputIter>
	path(InputIter first, InputIter last) : _pathname{first, last}
	{}

	path(path const & p) : _pathname{p._pathname} {}
	path(path && p) noexcept : _pathname{std::move(p._pathname)} {}
	path(string_type const & pathname) : _pathname{pathname} {}

	// assignments
	path & operator=(path const & p);

	// appends
	path & operator/=(path const & p);  //!< pripojenie

	// concatenation
	path & operator+=(path const & p);  //!< jednoduche spojenie

	// modifiers
	void clear();
	path & remove_filename();
	path & replace_extension(path const & newext = path{});
	void swap(path & rhs);

	// observers
	string_type const & native() const {return _pathname;}
	value_type const * c_str() const {return _pathname.c_str();}
	size_t size() const {return _pathname.size();}
	string_type const & string() const {return _pathname;}

	// decomposition
//	root_name();
	path root_directory() const;  //!< \note /home/ja/foo -> /
//	root_path();
//	relative_path();
	path parent_path() const;
	path filename() const;
//	stem();
	path extension() const;

	// query
	bool empty() const {return _pathname.empty();}
//	bool has_root_name() const;
	bool has_root_directory() const;
//	bool has_root_path() const;
//	bool has_relative_path() const;
	bool has_filename() const;
//	bool has_stem() const;
	bool has_extension() const;
	bool is_absolute() const {return has_root_directory();}
	bool is_relative() const {return !is_absolute();}

	// iterators
//	begin();
//	end();
//	rbegin();
//	rend();

	bool operator==(path const & p) const {return _pathname == p._pathname;}
	bool operator!=(path const & p) const {return _pathname != p._pathname;}

private:
	std::string _pathname;
};

inline path operator/(path const & lhs, path const & rhs) {return path{lhs} /= rhs;}

}}  // boost::filesystem
