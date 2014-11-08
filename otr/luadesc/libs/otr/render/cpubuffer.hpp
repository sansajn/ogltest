#pragma once
#include "render/buffer.hpp"

/*! @ingroup render */
class cpubuffer : public buffer
{
public:
	cpubuffer(void const * data = nullptr) : _data(data) {}
	~cpubuffer() {}
	void bind(int target) const override;
	void unbind(int target) const override {}
	void * data(int offset = 0) const override;

private:
	void const * _data;
};
