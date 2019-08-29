#include "jpegWorker.hpp"
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include <vector>
#include <jpeglib.h>

#include "base64.hpp"

#include <opencv2/opencv.hpp>
#include<opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#define uint8_t unsigned char

void decompressJPEG(const unsigned char *jpg_buffer, unsigned long jpg_size, std::string &result)
{

	// Variables for the decompressor itself
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	// Variables for the output buffer, and how long each row is
	unsigned long bmp_size;
	unsigned char *bmp_buffer;
	int row_stride, width, height, pixel_size;

    std::cout<<"Create Decompress struct"<<std::endl;
    // Allocate a new decompress struct, with the default error handler.
	// The default error handler will exit() on pretty much any issue,
	// so it's likely you'll want to replace it or supplement it with
	// your own.
	cinfo.err = jpeg_std_error(&jerr);	
	jpeg_create_decompress(&cinfo);

    std::cout<<"Set memory buffer as source"<<std::endl;
    // Configure this decompressor to read its data from a memory 
	// buffer starting at unsigned char *jpg_buffer, which is jpg_size
	// long, and which must contain a complete jpg already.
    jpeg_mem_src(&cinfo, jpg_buffer, jpg_size);

    // Have the decompressor scan the jpeg header. This won't populate
	// the cinfo struct output fields, but will indicate if the
	// jpeg is valid.
	int rc = jpeg_read_header(&cinfo, TRUE);

    if (rc != 1) {
		std::cout<<"File does not seem to be a normal JPEG"<<std::endl;
		return;
	}

    std::cout<<"Initiate JPEG decompression"<<std::endl;
    // By calling jpeg_start_decompress, you populate cinfo
	// and can then allocate your output bitmap buffers for
	// each scanline.
	jpeg_start_decompress(&cinfo);

    width = cinfo.output_width;
	height = cinfo.output_height;
	pixel_size = cinfo.output_components;

	std::cout<<"Image is "<<width<<" by "<<height<<" with "<<pixel_size<<" components"<<std::endl;

	bmp_size = width * height * pixel_size;
	bmp_buffer = (unsigned char*) malloc(bmp_size);

    // The row_stride is the total number of bytes it takes to store an
	// entire scanline (row). 
	row_stride = width * pixel_size;


	std::cout<<"Start reading scanlines"<<std::endl;

    //
	// Now that you have the decompressor entirely configured, it's time
	// to read out all of the scanlines of the jpeg.
	//
	// By default, scanlines will come out in RGBRGBRGB...  order, 
	// but this can be changed by setting cinfo.out_color_space
	//
	// jpeg_read_scanlines takes an array of buffers, one for each scanline.
	// Even if you give it a complete set of buffers for the whole image,
	// it will only ever decompress a few lines at a time. For best 
	// performance, you should pass it an array with cinfo.rec_outbuf_height
	// scanline buffers. rec_outbuf_height is typically 1, 2, or 4, and 
	// at the default high quality decompression setting is always 1.
	while (cinfo.output_scanline < cinfo.output_height) {
		unsigned char *buffer_array[1];
		buffer_array[0] = bmp_buffer + \
						   (cinfo.output_scanline) * row_stride;

		jpeg_read_scanlines(&cinfo, buffer_array, 1);

	}
	std::cout<<"Done reading scanlines"<<std::endl;

    // Once done reading *all* scanlines, release all internal buffers,
	// etc by calling jpeg_finish_decompress. This lets you go back and
	// reuse the same cinfo object with the same settings, if you
	// want to decompress several jpegs in a row.
	//
	// If you didn't read all the scanlines, but want to stop early,
	// you instead need to call jpeg_abort_decompress(&cinfo)
	jpeg_finish_decompress(&cinfo);

	// At this point, optionally go back and either load a new jpg into
	// the jpg_buffer, or define a new jpeg_mem_src, and then start 
	// another decompress operation.
	
	// Once you're really really done, destroy the object to free everything
	jpeg_destroy_decompress(&cinfo);
	
    // Write the decompressed bitmap out to a ppm file, just to make sure 
	// it worked. 
	int fd = open("output.ppm", O_CREAT | O_WRONLY, 0666);
	char buf[1024];

	rc = sprintf(buf, "P6 %d %d 255\n", width, height);
	write(fd, buf, rc); // Write the PPM image header before data
	write(fd, bmp_buffer, bmp_size); // Write out all RGB pixel data

    result = std::string(reinterpret_cast< char const* >(bmp_buffer));

	close(fd);
	free(bmp_buffer);

	std::cout<<"End of decompression, size = "<<bmp_size<<std::endl;

}

void compressJPEG(const std::vector<uint8_t>& input, const int width, const int height, int ratio, std::vector<uint8_t>& output)
{

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_ptr[1];
    int row_stride;

    uint8_t* outbuffer = NULL;
    uint64_t outlen = 0;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_mem_dest(&cinfo, &outbuffer, &outlen);

    // jrow is a libjpeg row of samples array of 1 row pointer
    cinfo.image_width = width & -1;
    cinfo.image_height = height & -1;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, ratio, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    std::vector<uint8_t> tmprowbuf(width * 3);

    JSAMPROW row_pointer[1];
    row_pointer[0] = &tmprowbuf[0];
    while (cinfo.next_scanline < cinfo.image_height) {
        //TODO
        /*unsigned i, j;
        unsigned offset = cinfo.next_scanline * cinfo.image_width * 2; //offset to the correct row
        for (i = 0, j = 0; i < cinfo.image_width * 2; i += 4, j += 6) { //input strides by 4 bytes, output strides by 6 (2 pixels)
            tmprowbuf[j + 0] = input[offset + i + 0]; // Y (unique to this pixel)
            tmprowbuf[j + 1] = input[offset + i + 1]; // U (shared between pixels)
            tmprowbuf[j + 2] = input[offset + i + 3]; // V (shared between pixels)
            tmprowbuf[j + 3] = input[offset + i + 2]; // Y (unique to this pixel)
            tmprowbuf[j + 4] = input[offset + i + 1]; // U (shared between pixels)
            tmprowbuf[j + 5] = input[offset + i + 3]; // V (shared between pixels)
        }*/
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    std::cout << "libjpeg produced " << outlen << " bytes" << std::endl;

    output = std::vector<uint8_t>(outbuffer, outbuffer + outlen);

}

/*std::string mat2str(const Mat &m)
{
    int params[3] = {0};
	params[0] = CV_IMWRITE_JPEG_QUALITY;
	params[1] = 100;

    std::vector<uint8_t> buf;
	bool code = cv::imencode(".jpg", m, buf, std::vector<int>(params, params+2));
	uint8_t* result = reinterpret_cast<uint8_t*> (&buf[0]);

	//return base64_encode(result, buf.size());
    return result;
}

Mat str2mat(const std::string& s)
{
	// Decode data
	std::string decoded_string = base64_decode(s);
	std::vector<uint8_t> data(decoded_string.begin(), decoded_string.end());

	cv::Mat img = imdecode(data, IMREAD_UNCHANGED);
	return img;
}*/

std::string getCompressedImage(std::string &bigImage, std::string &smallImage)
{
    std::string decoded_string = base64_decode(s);
	std::vector<uint8_t> data(decoded_string.begin(), decoded_string.end());

	cv::Mat img = imdecode(data, IMREAD_UNCHANGED);

    int params[3] = {0};
	params[0] = CV_IMWRITE_JPEG_QUALITY;
	params[1] = 60;

    std::vector<uint8_t> buf;
	bool code = cv::imencode(".jpg", m, buf, std::vector<int>(params, params+2));
	uint8_t* result = reinterpret_cast<uint8_t*> (&buf[0]);

	return base64_encode(result, buf.size());
}