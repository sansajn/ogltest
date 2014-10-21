#pragma once

/*! Data buffer abstraction.
@ingroup render */
class buffer
{
public:
	buffer() {}
	virtual ~buffer() {}

	class parameters
	{
	public:
		//! Sets the OpenGL state corresponding to these parameters.
		void set() const {}  // TODO: dummy implementation
		void unset() const {}  // TODO: dummy implementation
	};  // parameters

	virtual void bind(int target) const = 0;
	virtual void unbind(int target) const = 0;
	virtual void * data(int offset) const = 0;
};

