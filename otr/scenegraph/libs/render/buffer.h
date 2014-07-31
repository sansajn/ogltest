#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <GL/glew.h>

/*! Data buffer abstraction.
@ingroup render */
class buffer
{
public:
	buffer() {}
	virtual ~buffer() {}

	class parameters  //!< Wrapper okolo funkcie #glPixelStore.
	{
	public:
		parameters();

		//! Sets the OpenGL state corresponding to these parameters.
		void set() const;
		void unset() const;

		bool swap_bytes() const {return _swap_bytes;}  //!< true if little endian
		bool least_significant_bit_first() const {return _least_significant_bit_first;}
		GLint alignment() const {return _alignment;}
		GLsizei compressed_size() const {return _compressed_size;}
		glm::ivec3 subimage2D() const {return _sub_image2D;}
		glm::ivec2 subimage3D() const {return _sub_image3D;}

		parameters & swap_bytes(bool v);
		parameters & least_significant_bit_first(bool v);
		parameters & alignment(GLint v);
		parameters & compressed_size(GLsizei size);
		parameters & subimage2D(GLint skip_pixels, GLint skip_rows, GLint row_length);
		parameters & subimage3D(GLint skip_images, GLint image_height);

	private:
		bool _swap_bytes;
		bool _least_significant_bit_first;
		GLint _alignment;
		GLsizei _compressed_size;
		glm::ivec3 _sub_image2D;
		glm::ivec2 _sub_image3D;
		bool _modified;
	};  // parameters

	virtual void bind(int target) const = 0;
	virtual void unbind(int target) const = 0;
	virtual void * data(int offset = 0) const = 0;
};
