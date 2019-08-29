#include <iostream>
#include <string>
#include <vector>

#include "base64.hpp"
#include "jpegWorker.hpp"
#include "base64Img.hpp"
#include "fstream"

int main()
{
    /*std::cout<<"Decoding base64 image"<<std::endl;

    std::string decodedString = base64_decode(EncodedImg.c_str());

    long decodedLen = decodedString.length();
    std::cout<<"Decoded string length = "<<decodedLen<<std::endl;

    const unsigned char* constDecodedStr = reinterpret_cast<const unsigned char *>(decodedString.c_str()); 

    std::string decompressedStr;
    decompressJPEG(constDecodedStr, decodedLen, decompressedStr);

    std::vector<uint8_t> decomIn;
    std::vector<uint8_t> CompOut;
    decomIn.assign(decompressedStr.begin(), decompressedStr.end());

    compressJPEG(decomIn, width, height, ratio, CompOut);*/

    std::string smallImg;
    getCompressedImage(EncodedImg, smallImg);

    std::ofstream myfile;
    myfile.open ("base64Small.txt");
    myfile << "Writing this to a file.\n";
    myfile.close();

    return 0;
}