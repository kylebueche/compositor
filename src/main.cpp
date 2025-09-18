/*******************************************************
 * Author: Kyle Bueche
 * File: main.cpp
 * 
 * Image manipulator
 ******************************************************/
#include <iostream>
#include "image.h"
#include <limits>
#include <string>

int main()
{
    std::string readFilename;
    std::string writeFilename;
    std::cout << "Enter the name of the input image: ";
    std::cin >> readFilename;
    std::cout << "Enter the name of the output image file: ";
    std::cin >> writeFilename;
    ImageI image(readFilename.c_str());
    ImageF floatImage(image);
    floatImage.toGreyscale(0.30, 0.10, 0.60);
    ImageI resultImage(floatImage);
    resultImage.write(writeFilename.c_str());
	return 0;
}
