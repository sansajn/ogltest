#include <algorithm>
#include "label_gles2.hpp"

namespace ui {

using std::min;
using std::max;

// glyph helpers
static FT_BBox measure_glyphs(std::vector<FT_Glyph> const & glyphs);
static void copy(FT_BitmapGlyph glyph, FT_Vector const & pen, Magick::Image & result);

// bbox helpers
static unsigned width(FT_BBox const & b);
static unsigned height(FT_BBox const & b);
static void adjust(FT_BBox & result, FT_Pos dx);
static void unite(FT_BBox & result, FT_BBox const & bbox);

namespace detail {

Magick::Image render_glyphs(std::vector<FT_Glyph> const & glyphs)
{
	using namespace Magick;

	FT_BBox glyphs_bbox = measure_glyphs(glyphs);
	Image result{Geometry(width(glyphs_bbox) + glyphs_bbox.xMin, height(glyphs_bbox)), "black"};
	result.depth(8);

	FT_Vector pen{0, glyphs_bbox.yMax};

	for (FT_Glyph glyph : glyphs)
	{
		FT_BitmapGlyph glyph_bitmap = (FT_BitmapGlyph)glyph;

		FT_Error err = FT_Glyph_To_Bitmap((FT_Glyph *)&glyph_bitmap, FT_RENDER_MODE_NORMAL, nullptr, 0);
		assert(!err && "unable convert a glyph object to a glyph bitmap");

		copy(glyph_bitmap, pen, result);

		pen.x += glyph_bitmap->root.advance.x >> 16;  // 16.16
	}

	return result;
}

FT_Glyph load_glyph(unsigned char_code, FT_Face face)
{
	FT_UInt glyph_index = FT_Get_Char_Index(face, char_code);
	FT_Error err = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);  // now we can access face->glyph
	assert(!err && "can't load a glyph");

	FT_Glyph glyph = nullptr;
	err = FT_Get_Glyph(face->glyph, &glyph);
	assert(!err && "unable to extract a glyph object");

	return glyph;
}

}  // detail

FT_BBox measure_glyphs(std::vector<FT_Glyph> const & glyphs)
{
	if (glyphs.empty())
		return FT_BBox{0, 0, 0, 0};

	FT_BBox result;
	FT_Glyph_Get_CBox(glyphs[0], FT_GLYPH_BBOX_PIXELS, &result);

	FT_Pos advance = glyphs[0]->advance.x >> 16;  // 16.16

	for (int i = 1; i < glyphs.size(); ++i)
	{
		FT_BBox bbox;
		FT_Glyph_Get_CBox(glyphs[i], FT_GLYPH_BBOX_PIXELS, &bbox);
		adjust(bbox, advance);

		unite(result, bbox);

		advance += glyphs[i]->advance.x >> 16;  // 16.6
	}

	return result;
}

void copy(FT_BitmapGlyph glyph, FT_Vector const & pen, Magick::Image & result)
{
	using namespace Magick;

	FT_Bitmap & bitmap = glyph->bitmap;
	unsigned char * bmbuf = bitmap.buffer;

	if (bmbuf)  // some glyphs has no image data (spaces)
	{
		PixelPacket * pixels = result.getPixels(pen.x + glyph->left, pen.y - glyph->top, bitmap.width, bitmap.rows);
		assert(pixels && "out of image geometry");
		for (int i = 0; i < (bitmap.width*bitmap.rows); ++i, ++pixels, ++bmbuf)
			*pixels = ColorGray{*bmbuf/255.0};
		result.syncPixels();
	}
}

unsigned width(FT_BBox const & b)
{
	return b.xMax - b.xMin;
}

unsigned height(FT_BBox const & b)
{
	return b.yMax - b.yMin;
}

void unite(FT_BBox & result, FT_BBox const & bbox)
{
	result.xMin = min(result.xMin, bbox.xMin);
	result.xMax = max(result.xMax, bbox.xMax);
	result.yMin = min(result.yMin, bbox.yMin);
	result.yMax = max(result.yMax, bbox.yMax);
}

void adjust(FT_BBox & result, FT_Pos dx)
{
	result.xMin += dx;
	result.xMax += dx;
}

}  // ui
