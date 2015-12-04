#include "pix_jpeg.hpp"
#include <csetjmp>
#include <cassert>
#include <jpeglib.h>

using std::string;

namespace pix {

struct setjmp_error_mgr
{
	jpeg_error_mgr sem;  //!< standard error manager
	jmp_buf setjmp_buffer;
};

static void longjmp_error_exit(j_common_ptr cinfo);


void jpeg_decoder::decode(string const & fname)
{
	FILE * fin = fopen(fname.c_str(), "rb");
	assert(fin);

	jpeg_decompress_struct cinfo;

	// error handling
	setjmp_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr.sem);
	jerr.sem.error_exit = longjmp_error_exit;

	if (setjmp(jerr.setjmp_buffer))
	{
		assert(0 && "jpeg internal error");
		char what[JMSG_LENGTH_MAX];
		(*jerr.sem.format_message)((j_common_ptr)(&cinfo), what);  // generate an error message

		jpeg_destroy_decompress(&cinfo);  // cleanup
		fclose(fin);

		throw decoder_exception{what};
	}

	// read jpeg
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, fin);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	static_assert(BITS_IN_JSAMPLE == 8, "supports only 8-bit jpeg");
	static_assert(std::is_same<JSAMPLE, uint8_t>::value, "JSAMPLE is not uint8_t, pixels buffer type not match");

	int rowbytes = cinfo.output_width * cinfo.output_components;
	uint8_t * pixels = new uint8_t[rowbytes * cinfo.output_height];  // chcem precitat vsetko do jedneho buffer-u

	uint8_t ** rows = new uint8_t * [cinfo.output_height];
	for (unsigned r = 0; r <cinfo.output_height; ++r)
		rows[r] = pixels + r*rowbytes;

	while (cinfo.output_scanline < cinfo.output_height)
	{
		JSAMPARRAY dst = rows + cinfo.output_scanline;  // byte **
		jpeg_read_scanlines(&cinfo, dst, cinfo.output_height);  // pokus sa precitat cely obrazok
	}

	result.width = cinfo.output_width;
	result.height = cinfo.output_height;
	result.rowbytes = rowbytes;
	result.channels = cinfo.output_components;
	result.depth = 1;  // only 8-bit for channel
	result.pixels = pixels;

	// clenup
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	delete [] rows;
	fclose(fin);
}


void longjmp_error_exit(j_common_ptr cinfo)
{
	setjmp_error_mgr * mgr = (setjmp_error_mgr *)cinfo->err;
	longjmp(mgr->setjmp_buffer, 1);  // return control to the setjmp point
}

}  // pix
