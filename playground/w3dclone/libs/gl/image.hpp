#pragma once
#include <string>
#include <memory>
#include <glm/matrix.hpp>
#include "program.hpp"
#include "texture.hpp"

namespace ui {

//! Kontrolka zobraziaca obrazok v okne.
class image
{
public:
	image() {}  //!< just for move assignment purpose (do not use)
	image(std::string const & file_name);
	void render();
	void transform(glm::mat4 const & t);  //!< image transformation
	unsigned width() const {return _image_tex.width();}
	unsigned height() const {return _image_tex.height();}

private:
	glm::mat4 _t;
	texture2d _image_tex;
	shader::program _prog;
	std::shared_ptr<shader::uniform> _sampler_u;
	std::shared_ptr<shader::uniform> _transform_u;
};

}  // ui
