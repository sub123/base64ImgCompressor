#include <string>
#include <vector>

void decompressJPEG(const unsigned char *jpg_buffer, unsigned long jpg_size, std::string &result);

void compressJPEG(const std::vector<uint8_t>& input, const int width, const int height, int ratio, std::vector<uint8_t>& output);

std::string getCompressedImage(std::string &bigImage, std::string &smallImage);