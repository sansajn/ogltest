#pragma once
#include <string>
#include <GL/glew.h>
#include "core/ptr.hpp"
#include "render/types.hpp"
#include "render/any_value.hpp"

class program;

/*! Modeluje uniform premennú.
Nastavenie premennej sa prejaví iba v programe, ktorí ju používa.
\ingroup render */
class uniform
{
public:
	virtual ~uniform() {}
	virtual uniform_type type() const = 0;
	std::string name() const {return _name;}
	virtual void set_value(ptr<any_value>) = 0;

protected:
	uniform(program * prog, std::string const & name, GLint location) : _prog(prog), _name(name), _location(location) {}
	virtual void set_value() = 0;
	
	program * _prog;
	std::string _name;
	GLuint _location;

	friend class program;  // modifikuje _prog
};


template <uniform_type U, typename T>
class uniform1 : public uniform
{
public:
	~uniform1() {}
	uniform_type type() const override {return U;}
	T get() const {return _value;}

	void set(T value)
	{
		_value = value;
		if (_prog)  // BUG: ak nie je use-nuty ziaden program tak openGL vygeneruje error
			set_value();
	}

	void set_value(ptr<any_value> v) override {set(std::dynamic_pointer_cast<any_value1<U, T>>(v)->get());}

protected:
	uniform1(program * prog, std::string const & name, GLint location)
		: uniform(prog, name, location)
	{}

	void set_value() override;

private:
	T _value;

	friend class program;
};

template <uniform_type U, typename T>
class uniform2 : public uniform
{
public:
	typedef glm::detail::tvec2<T, glm::precision::highp> vec_type;

	~uniform2() {}
	uniform_type type() const override {return U;}
	vec_type get() const {return _value;}

	void set(vec_type const & value)
	{
		_value = value;
		if (_prog)
			set_value();
	}

	void set_value(ptr<any_value> v) override {set(std::dynamic_pointer_cast<any_value2<U, T>>(v)->get());}

protected:
	uniform2(program * prog, std::string const & name, GLint location)
		: uniform(prog, name, location)
	{}

	void set_value() override;

private:
	vec_type _value;

	friend class program;
};

template <uniform_type U, typename T>
class uniform3 : public uniform
{
public:
	typedef glm::detail::tvec3<T, glm::precision::highp> vec_type;

	~uniform3() {}
	uniform_type type() const override {return U;}
	vec_type get() const {return _value;}

	void set(vec_type const & value)
	{
		_value = value;
		if (_prog)
			set_value();
	}

	void set_value(ptr<any_value> v) override {set(std::dynamic_pointer_cast<any_value3<U, T>>(v)->get());}

protected:
	uniform3(program * prog, std::string const & name, GLint location)
		: uniform(prog, name, location)
	{}

	void set_value() override;

private:
	vec_type _value;

	friend class program;
};

template <uniform_type U, typename T>
class uniform4 : public uniform
{
public:
	typedef glm::detail::tvec4<T, glm::precision::highp> vec_type;

	~uniform4() {}
	uniform_type type() const override {return U;}
	vec_type get() const {return _value;}

	void set(vec_type const & value)
	{
		_value = value;
		if (_prog)
			set_value();
	}

	void set_value(ptr<any_value> v) override {set(std::dynamic_pointer_cast<any_value4<U, T>>(v)->get());}

protected:
	uniform4(program * prog, std::string const & name, GLint location)
		: uniform(prog, name, location)
	{}

	void set_value() override;

private:
	vec_type _value;

	friend class program;
};

typedef uniform1<uniform_type::vec1f, GLfloat> uniform1f;
typedef uniform1<uniform_type::vec1d, GLdouble> uniform1d;
typedef uniform1<uniform_type::vec1i, GLint> uniform1i;
typedef uniform1<uniform_type::vec1ui, GLuint> uniform1ui;
typedef uniform1<uniform_type::vec1b, bool> uniform1b;
typedef uniform2<uniform_type::vec2f, GLfloat> uniform2f;
typedef uniform2<uniform_type::vec2d, GLdouble> uniform2d;
typedef uniform2<uniform_type::vec2i, GLint> uniform2i;
typedef uniform2<uniform_type::vec2ui, GLuint> uniform2ui;
typedef uniform2<uniform_type::vec2b, bool> uniform2b;
typedef uniform3<uniform_type::vec3f, GLfloat> uniform3f;
typedef uniform3<uniform_type::vec3d, GLdouble> uniform3d;
typedef uniform3<uniform_type::vec3i, GLint> uniform3i;
typedef uniform3<uniform_type::vec3ui, GLuint> uniform3ui;
typedef uniform3<uniform_type::vec3b, bool> uniform3b;
typedef uniform4<uniform_type::vec4f, GLfloat> uniform4f;
typedef uniform4<uniform_type::vec4d, GLdouble> uniform4d;
typedef uniform4<uniform_type::vec4i, GLint> uniform4i;
typedef uniform4<uniform_type::vec4ui, GLuint> uniform4ui;
typedef uniform4<uniform_type::vec4b, bool> uniform4b;


template <uniform_type U, typename T, int C, int R>
class uniform_matrix : public uniform
{
public:
	~uniform_matrix() {}
	uniform_type type() const override {return U;}

	T const * get() const {return _value;}

	void set(T const * value)
	{
		for (int i = 0; i < R*C; ++i)
			_value[i] = value[i];
		if (_prog)
			set_value();
	}

	void set_value(ptr<any_value> v) {set(std::dynamic_pointer_cast<any_value_matrix<U, T, C, R>>(v)->get());}

protected:
	uniform_matrix(program * prog, std::string const & name, GLint location, int stride, int is_row_major)
		: uniform(prog, name, location), _stride(stride), _row_major(is_row_major)
	{}

	void set_value() override;

	T _value[C*R];

private:
	int _stride;
	int _row_major;

	friend class program;
};

template <uniform_type U, typename T>
class uniform_matrix3 : public uniform_matrix<U, T, 3, 3>
{
public:
	typedef glm::detail::tmat3x3<T, glm::precision::highp> mat_type;
	typedef uniform_matrix<U, T, 3, 3> base;

	mat_type get_matrix() const {return *(mat_type *)base::get();}
	void set_matrix(mat_type const & value) {base::set(glm::value_ptr(value));}

protected:
	uniform_matrix3(program * prog, std::string const & name, GLint location, int stride, int is_row_major)
		: base(prog, name, location, stride, is_row_major)
	{}

	friend class program;
};

template <uniform_type U, typename T>
class uniform_matrix4 : public uniform_matrix<U, T, 4, 4>
{
public:
	typedef glm::detail::tmat4x4<T, glm::precision::highp> mat_type;
	typedef uniform_matrix<U, T, 4, 4> base;

	mat_type get_matrix() const {return *(mat_type *)base::get();}
	void set_matrix(mat_type const & value) {base::set(glm::value_ptr(value));}

protected:
	uniform_matrix4(program * prog, std::string const & name, GLint location, int stride, int is_row_major)
		: base(prog, name, location, stride, is_row_major)
	{}

	friend class program;
};

typedef uniform_matrix<uniform_type::mat2f, GLfloat, 2, 2> uniform_matrix2f;
typedef uniform_matrix<uniform_type::mat2d, GLdouble, 2, 2> uniform_matrix2d;
typedef uniform_matrix3<uniform_type::mat3f, GLfloat> uniform_matrix3f;
typedef uniform_matrix3<uniform_type::mat3d, GLdouble> uniform_matrix3d;
typedef uniform_matrix4<uniform_type::mat4f, GLfloat> uniform_matrix4f;
typedef uniform_matrix4<uniform_type::mat4d, GLdouble> uniform_matrix4d;
typedef uniform_matrix<uniform_type::mat2x3f, GLfloat, 2, 3> uniform_matrix2x3f;
typedef uniform_matrix<uniform_type::mat2x3d, GLdouble, 2, 3> uniform_matrix2x3d;
typedef uniform_matrix<uniform_type::mat2x4f, GLfloat, 2, 4> uniform_matrix2x4f;
typedef uniform_matrix<uniform_type::mat2x4d, GLdouble, 2, 4> uniform_matrix2x4d;
typedef uniform_matrix<uniform_type::mat3x2f, GLfloat, 3, 2> uniform_matrix3x2f;
typedef uniform_matrix<uniform_type::mat3x2d, GLdouble, 3, 2> uniform_matrix3x2d;
typedef uniform_matrix<uniform_type::mat3x4f, GLfloat, 3, 4> uniform_matrix3x4f;
typedef uniform_matrix<uniform_type::mat3x4d, GLdouble, 3, 4> uniform_matrix3x4d;
typedef uniform_matrix<uniform_type::mat4x2f, GLfloat, 4, 2> uniform_matrix4x2f;
typedef uniform_matrix<uniform_type::mat4x2d, GLdouble, 4, 2> uniform_matrix4x2d;
typedef uniform_matrix<uniform_type::mat4x3f, GLfloat, 4, 3> uniform_matrix4x3f;
typedef uniform_matrix<uniform_type::mat4x3d, GLdouble, 4, 3> uniform_matrix4x3d;


class sampler;
class texture;

class uniform_sampler : public uniform
{
public:
	~uniform_sampler() {}
	uniform_type type() const override {return _type;}
	ptr<sampler> get_sampler() const {return _sampler;}
	void set_sampler(ptr<sampler> s);
	ptr<texture> get() const {return _value;}
	void set(ptr<texture> value);
	void set_value(ptr<any_value> v) override;

protected:
	uniform_sampler(uniform_type type, program * prog, std::string const & name, GLint location)
		: uniform(prog, name, location), _type(type), _unit(-1)
	{}

	void set_value() override;

private:
	uniform_type _type;
	ptr<sampler> _sampler;
	ptr<texture> _value;
	int _unit;  //!< the current texture unit value of this uniform

	friend class program;
};
