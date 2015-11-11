// kopirovanie do texture 2d array-u
#include <string>
#include <cstring>
#include <Magick++.h>

using std::string;

string const texture_path = "sswvi0.png";


void copy(uint8_t const * src, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, uint8_t * dst)
{
	for (int r = 0; r < src_h; ++r)
	{
		uint8_t * d = dst + ((r+dst_y) * dst_w + dst_x)*4;
		for (int c = 0; c < src_w; ++c)
		{
			*(d++) = *(src++);
			*(d++) = *(src++);
			*(d++) = *(src++);
			*(d++) = *(src++);
		}
	}
}

void copy32(void const * src, int srcw, int srch, int dstx, int dsty, int dstw, int dsth, void * dst)
{
	uint32_t const * s = (uint32_t const *)src;
	for (int r = 0; r < srch; ++r)
	{
		uint32_t * d = (uint32_t *)dst + ((r+dsty) * dstw + dstx);
		for (int c = 0; c < srcw; ++c)
			*(d++) = *(s++);
	}
}

int main(int argc, char * argv[])
{
	Magick::Image im{texture_path.c_str()};
	im.flip();
	Magick::Blob imblob;
	im.write(&imblob, "RGBA");

	int width = 64, height = 64;
	int x = (width - im.columns())/2;
	int y = (height - im.rows())/2;
	uint8_t * data = new uint8_t[width*height*4];
	memset(data, 0, width*height*4);
//	copy((uint8_t const *)imblob.data(), im.columns(), im.rows(), x, y, width, height, data);
	copy32(imblob.data(), im.columns(), im.rows(), x, y, width, height, data);

	Magick::Image iout;
	iout.read(width, height, "RGBA", Magick::StorageType::CharPixel, data);
	iout.flip();
	iout.write("out.png");

	delete [] data;

	return 0;
}
