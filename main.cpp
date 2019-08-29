#include <iostream>
#include <string>
#include <vector>

#include "base64.hpp"
#include "jpegWorker.hpp"
#include "base64Img.hpp"

int main()
{
    std::cout<<"Decoding base64 image"<<std::endl;

    std::string decodedString = base64_decode(EncodedImg.c_str());

    long decodedLen = decodedString.length();
    std::cout<<"Decoded string length = "<<decodedLen<<std::endl;

    const unsigned char* constDecodedStr = reinterpret_cast<const unsigned char *>(decodedString.c_str()); 

    decompressJPEG(constDecodedStr, decodedLen);

    return 0;
}