/********************************************
 * Author: Kyle Bueche
 * File: image.cpp
 *
 *******************************************/

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>
#include "image.h"

/************************************************
 * This is kind of terrible but its okay.
 *
 * The full image is copied, this would be meh
 * for loading a large sequence of images.
 *
 ***********************************************/

// From file constructor

ImageF::ImageF()
{
    this->pixels = NULL;
    std::cout << "vtable: " << *(void**)this << std::endl;
}

ImageF::ImageF(const char *filename)
{
    this->pixels = NULL;
    this->buffer(filename);
}

// Copy constructor
ImageF::ImageF(const ImageF& img)
{
    this->pixels = NULL;
    this->buffer(img);
}

ImageF::~ImageF()
{
    if (pixels != NULL)
    {
        delete(pixels);
    }
}

// From file bufferer
ImageF::buffer(const char *filename)
{
    std::cout << "vtable: " << *(void**)this << std::endl;
    int lwidth;
    int lheight;
    int lpixelCount;
    int lnrChannels;

    // Desired channels is 4, will always convert to an rgba image.
    unsigned char *data = stbi_load(filename, &lwidth, &lheight, &lnrChannels, NUM_CHANNELS);
    int dataIndex = 0;


    // Handle stbi loading errors
    if (data)
    {
        lpixelCount = lwidth * lheight;
        if (pixels == NULL)
        {
            pixels = new pixel4f_t[lpixelCount];
        }
        else
        {
            // Don't reallocate if the new image is the same size as this buffer
            if (lpixelCount != this->pixelCount)
            {
                delete(pixels);
                pixels = new pixel4f_t[lpixelCount];
            }
        }
        std::cout << "Width: " << lwidth << std::endl;
        std::cout << "Height: " << lheight << std::endl;
        std::cout << "Width * Height: " << lwidth * lheight << std::endl;
        std::cout << "Pixel count: " << lpixelCount << std::endl;
        void* t = *(void**)this;
        // Handle dynamic allocation failure
        if (pixels != NULL)
        {
            std::cout << "HIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII: " << *(void**)this << std::endl;
            std::cout << "vtable: " << *(void**)this << std::endl;
            float scalar = 1.0f / 255.0f;
            std::cout << "pixel count: " << lpixelCount <<std::endl;
            for (int i = 0; i < lpixelCount; i++)
            {
                dataIndex = i * 4;
                pixels[i].col.r = scalar * float(data[dataIndex]);
                pixels[i].col.g = scalar * float(data[dataIndex + 1]);
                pixels[i].col.b = scalar * float(data[dataIndex + 2]);
                pixels[i].col.a = scalar * float(data[dataIndex + 3]);
                if (*(void**)this != t)
                {
                    std::cout << "CORRUPTION AT: " << i << std::endl;
                }
            }
            this->width = lwidth;
            this->height = lheight;
            this->pixelCount = lpixelCount;
            this->aspectRatio = lwidth / lheight;
            std::cout << "vtable: " << *(void**)this << std::endl;
        }
        else
        {
            std::cerr << "ERROR: Couldn't allocate memory for the image";
        }
        stbi_image_free(data);
    }
    else
    {
        std::cerr << "ERROR: STBI Failed to load the image" << std::endl;
    }
    std::cout << "hi2" << std::endl;
    std::cout << "vtable: " << *(void**)this << std::endl;
}

// Copy bufferer
ImageF::buffer(const ImageF& img)
{
    if (pixels == NULL)
    {
        this->pixels = new pixel4f_t[img.pixelCount];
    }
    else
    {
        // Don't reallocate if the new image is the same size as this buffer
        if (img.pixelCount != this->pixelCount)
        {
            delete(pixels);
            this->pixels = new pixel4f_t[img.pixelCount];
        }
    }
    if (pixels != NULL)
    {
        this->width = img.width;
        this->height = img.height;
        this->pixelCount = img.pixelCount;
        this->aspectRatio = img.aspectRatio;
        for (int i = 0; i < pixelCount; i++)
        {
            this->pixels[i] = img.pixels[i];
        }
    }
    else
    {
        std::cerr << "ERROR: Couldn't allocate memory for the image";
    }
}

void ImageF::write(const char *filename)
{
    pixel4i_t *intBuffer = new pixel4i_t[pixelCount];
    for (int i = 0; i < pixelCount; i++)
    {
        intBuffer[i].col.r = std::min(uint8_t(255.99f * intBuffer[i].col.r), uint8_t(255));
        intBuffer[i].col.g = std::min(uint8_t(255.99f * intBuffer[i].col.g), uint8_t(255));
        intBuffer[i].col.b = std::min(uint8_t(255.99f * intBuffer[i].col.b), uint8_t(255));
        intBuffer[i].col.a = std::min(uint8_t(255.99f * intBuffer[i].col.a), uint8_t(255));
    }
    stbi_write_png(filename, this->width, this->height, NUM_CHANNELS, intBuffer, this->width * sizeof(uint8_t) * NUM_CHANNELS);
    delete(intBuffer);
}

void ImageF::toGreyscale(float rScalar, float gScalar, float bScalar)
{
    std::cout << "1";
    float avg;
    for (int i = 0; i < pixelCount; i++)
    {
        avg = rScalar * pixels[i].col.r
            + gScalar * pixels[i].col.g
            + bScalar * pixels[i].col.b;
        pixels[i].col.r = avg;
        pixels[i].col.g = avg;
        pixels[i].col.b = avg;
    }
}

void ImageF::toNegative()
{
    for (int i = 0; i < pixelCount; i++)
    {
        pixels[i].col.r = 1.0 - pixels[i].col.r;
        pixels[i].col.g = 1.0 - pixels[i].col.g;
        pixels[i].col.b = 1.0 - pixels[i].col.b;
    }
}
