#pragma once
#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include "core/ptr.hpp"
#include "render/texture.hpp"
#include "render/types.hpp"

/*! Implementuje všeobecnú hodnotu.
\ingroup render */
class any_value
{
public:
	virtual ~any_value() {}
	virtual uniform_type type() const = 0;
	std::string const & name() const {return _name;}

protected:
	any_value(std::string const & name)	: _name(name) {}

	std::string _name;
};

template <uniform_type U, typename T>
class any_value1 : public any_value
{
public:
	any_value1(std::string const & name) : any_value(name) {}
	any_value1(std::string const & name, T value) : any_value(name), _value(value) {}
	virtual ~any_value1() {}
	uniform_type type() const override {return U;}
	T get() const {return _value;}
	void set(T value) {_value = value;}

private:
	T _value;
};

template <uniform_type U, typename T>
class any_value2 : public any_value
{
public:
	typedef glm::detail::tvec2<T, glm::precision::highp> vec_type;

	any_value2(std::string const & name) : any_value(name) {}
	any_value2(std::string const & name, vec_type const & value) : any_value(name), _value(value) {}
	virtual ~any_value2() {}
	uniform_type type() const override {return U;}
	vec_type get() const {return _value;}
	void set(vec_type const & value) {_value = value;}

private:
	vec_type _value;
};

template <uniform_type U, typename T>
class any_value3 : public any_value
{
public:
	typedef glm::detail::tvec3<T, glm::precision::highp> vec_type;

	any_value3(std::string const & name) : any_value(name) {}
	any_value3(std::string const & name, vec_type const & value) : any_value(name), _value(value) {}
	virtual ~any_value3() {}
	uniform_type type() const override {return U;}
	vec_type get() const {return _value;}
	void set(vec_type const & value) {_value = value;}

private:
	vec_type _value;
};

template <uniform_type U, typename T>
class any_value4 : public any_value
{
public:
	typedef glm::detail::tvec4<T, glm::precision::highp> vec_type;

	any_value4(std::string const & name) : any_value(name) {}
	any_value4(std::string const & name, vec_type const & value) : any_value(name), _value(value) {}
	virtual ~any_value4() {}
	uniform_type type() const override {return U;}
	vec_type get() const {return _value;}
	void set(vec_type const & value) {_value = value;}

private:
	vec_type _value;
};

typedef any_value1<uniform_type::vec1f, GLfloat> any_value1f;
typedef any_value1<uniform_type::vec1d, GLdouble> any_value1d;
typedef any_value1<uniform_type::vec1i, GLint> any_value1i;
typedef any_value1<uniform_type::vec1ui, GLuint> any_value1ui;
typedef any_value1<uniform_type::vec1b, bool> any_value1b;
typedef any_value2<uniform_type::vec2f, GLfloat> any_value2f;
typedef any_value2<uniform_type::vec2d, GLdouble> any_value2d;
typedef any_value2<uniform_type::vec2i, GLint> any_value2i;
typedef any_value2<uniform_type::vec2ui, GLuint> any_value2ui;
typedef any_value2<uniform_type::vec2b, bool> any_value2b;
typedef any_value3<uniform_type::vec3f, GLfloat> any_value3f;
typedef any_value3<uniform_type::vec3d, GLdouble> any_value3d;
typedef any_value3<uniform_type::vec3i, GLint> any_value3i;
typedef any_value3<uniform_type::vec3ui, GLuint> any_value3ui;
typedef any_value3<uniform_type::vec3b, bool> any_value3b;
typedef any_value4<uniform_type::vec4f, GLfloat> any_value4f;
typedef any_value4<uniform_type::vec4d, GLdouble> any_value4d;
typedef any_value4<uniform_type::vec4i, GLint> any_value4i;
typedef any_value4<uniform_type::vec4ui, GLuint> any_value4ui;
typedef any_value4<uniform_type::vec4b, bool> any_value4b;


template <uniform_type U, typename T, int C, int R>
class any_value_matrix : public any_value
{
public:
	any_value_matrix(std::string const & name) : any_value(name) {}
	
	any_value_matrix(std::string const & name, T const * value)
		: any_value(name) 
	{
		set(value);
	}

	~any_value_matrix() {}
	uniform_type type() const override {return U;}
	T const * get() const {return _value;}

	void set(T const * value)
	{
		for (int i = 0; i < R*C; ++i)
			_value[i] = value[i];
	}

protected:
	T _value[R*C];
};

template <uniform_type U, typename T>
class any_value_matrix3 : public any_value_matrix<U, T, 3, 3>
{
public:
	typedef any_value_matrix<U, T, 3, 3> base;
	typedef glm::detail::tmat3x3<T, glm::precision::highp> mat_type;

	any_value_matrix3(std::string const & name) : base(name) {}	
	any_value_matrix3(std::string const & name, mat_type const & v) : base(name) {set_matrix(v);}
	mat_type get_matrix() const {return *(mat_type *)(base::get());}
	void set_matrix(mat_type const & v) {base::set(glm::value_ptr(v));}
};

template <uniform_type U, typename T>
class any_value_matrix4 : public any_value_matrix<U, T, 4, 4>
{
public:
	typedef any_value_matrix<U, T, 4, 4> base;
	typedef glm::detail::tmat4x4<T, glm::precision::highp> mat_type;

	any_value_matrix4(std::string const & name) : base(name) {}
	any_value_matrix4(std::string const & name, mat_type const & v) : base(name) {set_matrix(v);}
	mat_type get_matrix() const {return *(mat_type *)(base::get());}
	void set_matrix(mat_type const & v) {base::set(glm::value_ptr(v));}
};


typedef any_value_matrix<uniform_type::mat2f, GLfloat, 2, 2> any_value_matrix2f;
typedef any_value_matrix<uniform_type::mat2d, GLdouble, 2, 2> any_value_matrix2d;
typedef any_value_matrix3<uniform_type::mat3f, GLfloat> any_value_matrix3f;
typedef any_value_matrix3<uniform_type::mat3d, GLdouble> any_value_matrix3d;
typedef any_value_matrix4<uniform_type::mat4f, GLfloat> any_value_matrix4f;
typedef any_value_matrix4<uniform_type::mat4d, GLdouble> any_value_matrix4d;
typedef any_value_matrix<uniform_type::mat2x3f, GLfloat, 2, 3> any_value_matrix2x3f;
typedef any_value_matrix<uniform_type::mat2x3d, GLdouble, 2, 3> any_value_matrix2x3d;
typedef any_value_matrix<uniform_type::mat2x4f, GLfloat, 2, 4> any_value_matrix2x4f;
typedef any_value_matrix<uniform_type::mat2x4d, GLdouble, 2, 4> any_value_matrix2x4d;
typedef any_value_matrix<uniform_type::mat3x2f, GLfloat, 3, 2> any_value_matrix3x2f;
typedef any_value_matrix<uniform_type::mat3x2d, GLdouble, 3, 2> any_value_matrix3x2d;
typedef any_value_matrix<uniform_type::mat3x4f, GLfloat, 3, 4> any_value_matrix3x4f;
typedef any_value_matrix<uniform_type::mat3x4d, GLdouble, 3, 4> any_value_matrix3x4d;
typedef any_value_matrix<uniform_type::mat4x2f, GLfloat, 4, 2> any_value_matrix4x2f;
typedef any_value_matrix<uniform_type::mat4x2d, GLdouble, 4, 2> any_value_matrix4x2d;
typedef any_value_matrix<uniform_type::mat4x3f, GLfloat, 4, 3> any_value_matrix4x3f;
typedef any_value_matrix<uniform_type::mat4x3d, GLdouble, 4, 3> any_value_matrix4x3d;


class any_value_sampler : public any_value
{
public:
	any_value_sampler(uniform_type type, std::string const & name) : any_value(name), _type(type) {}
	any_value_sampler(uniform_type type, std::string const & name, ptr<texture> value) : any_value(name), _type(type), _value(value) {}

	~any_value_sampler() {}
	uniform_type type() const override {return _type;}
	ptr<texture> get() const {return _value;}
	void set(ptr<texture> value) {_value = value;}

private:
	uniform_type _type;
	ptr<texture> _value;
};
