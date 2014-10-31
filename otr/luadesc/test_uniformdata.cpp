class uniform_data
{
public:
	void get(void * value);
	void set(void const * value);
};

template <T>
class uniform_data_impl : uniform_data
{};


float, vec2, vec3, vec4

class uniform_data_impl<float> : uniform_data
{
public:
	void get(void * value)
	{
		uniform_variable<float> u = ...;
		*(float *)value = u.value();
	}
};

void getcallback(void * value, void * client_data)
{
	uniform_data * ud = static_cast<uniform_data *>(client_date);	
	ud->value(value);	
}

int main(int argc, char * argv[])
{
	return 0;
}


class uniform_data
{
public:
	uniform_data() {}
	virtual ~uniform_data() {}
	virtual void get(void * value) const = 0;
	virtual void set(void const * value) = 0;

private:
	uniform_variable uniform() const;
};

template <typename T>
struct uniform_data_impl : public uniform_data
{};

struct uniform_data_impl<float> : public uniform_data 
{
	using uniform_data::uniform_data;

	void get(void * value) const override {
		*(float *)value = uniform_cast<float>(uniform());
	}

	void set(void const * value) override {
		uniform() = *(float *)value;
	}
};

template <>
struct uniform_data_impl<glm::vec3> : public uniform_data
{
	using uniform_data::uniform_data;

	void get(void * value) const override {
		*(glm::vec3 *)value = uniform_cast<glm::vec3>(uniform());
	}
};


// stavové riešenie

uniform_data
{
public:
	enum data_type {float_t, vec2_t, vec3_t, vec4_t};
	
	uniform_data(data_type type) : _type(uniform_type)
	{}

	void get(void * value)
	{
		ptr<module> m = ...;
		ptr<shader_program> p = m.users()->begin();
		uniform_variable u(name, *p);

		if (_type == data_type::float_t)
			*(float *)value = uniform_cast<float>(u);			
		else if (_type == data_type::vec2_t)
			*(glm::vec2)value = uniform_cast<glm::vec2>(u);
		else if (_type == data_type::vec3_t)
			*(glm::vec3)value = uniform_cast<glm::vec3>(u);
		else if (_type == data_type::vec4_t)
			*(glm::vec4)value = uniform_cast<glm::vec4>(u);
		else
			assert(false && "unsupported variable type");
	}

private:
	int _type;
}



