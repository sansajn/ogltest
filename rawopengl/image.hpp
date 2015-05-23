#pragma once
#include <string>
#include <memory>
#include "program.hpp"
#include "texture.hpp"

namespace ui {

//! Kontrolka zobraziaca obrazok v okne.
class image
{
public:
	image() {}
	image(std::string const & file_name);
	void render();

private:
	texture2d _image_tex;
	shader::program _prog;
	std::shared_ptr<shader::uniform> _sampler_u;
};

}  // ui
