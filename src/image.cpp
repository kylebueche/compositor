/********************************************
 * Author: Kyle Bueche
 * File: image.cpp
 *
 *******************************************/

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include "image.h"

/************************************************
 * This is kind of terrible but its okay.
 *
 * The full image is copied, this would be meh
 * for loading a large sequence of images.
 *
 ***********************************************/
ImageI::ImageI(const char *fileName)
{
    pixels = NULL;
    int width;
    int height;
    int pixelCount;
    int nrChannels;

    // Desired channels is 4, will always convert to an rgba image.
    unsigned char *data = stbi_load(fileName, &width, &height, &nrChannels, DESIRED_CHANNELS);
    int dataIndex;

    if (data)
    {
        pixelCount = width * height;
        this->pixels = new pixel4i_t[pixelCount];
        if (pixels != NULL)
        {
            for (int i = 0; i < pixelCount; i++)
            {
                dataIndex = i * 4;
                this->pixels[i].col.r = uint8_t(data[dataIndex]);
                this->pixels[i].col.g = uint8_t(data[dataIndex + 1]);
                this->pixels[i].col.b = uint8_t(data[dataIndex + 2]);
                this->pixels[i].col.a = uint8_t(data[dataIndex + 3]);
            }
            this->width = width;
            this->height = height;
            this->pixelCount = pixelCount;
            this->aspectRatio = width / height;
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
}

ImageI::~ImageI()
{
    if (pixels != NULL)
    {
        delete(pixels);
    }
}

// Copy int to int
ImageI::ImageI(const ImageI& img)
{
    this->pixels = new pixel4i_t[img.width * img.height];
    this->width = img.width;
    this->height = img.height;
    this->pixelCount = img.pixelCount;
    this->aspectRatio = img.aspectRatio;
    for (int i = 0; i < img.width * img.height; i++)
    {
        this->pixels[i] = img.pixels[i];
    }
}

// Copy float to int
ImageI::ImageI(const ImageF& img)
{
    this->pixels = new pixel4i_t[img.width * img.height];
    this->width = img.width;
    this->height = img.height;
    this->pixelCount = img.pixelCount;
    this->aspectRatio = img.aspectRatio;
    for (int i = 0; i < pixelCount; i++)
    {
        this->pixels[i].col.r = std::min(uint8_t(255.99f * img.pixels[i].col.r), uint8_t(255));
        this->pixels[i].col.g = std::min(uint8_t(255.99f * img.pixels[i].col.g), uint8_t(255));
        this->pixels[i].col.b = std::min(uint8_t(255.99f * img.pixels[i].col.b), uint8_t(255));
        this->pixels[i].col.a = std::min(uint8_t(255.99f * img.pixels[i].col.a), uint8_t(255));
    }
}

// Copy float to float
ImageF::ImageF(const ImageF& img)
{
    this->pixels = new pixel4f_t[img.width * img.height];
    this->width = img.width;
    this->height = img.height;
    this->pixelCount = img.pixelCount;
    this->aspectRatio = img.aspectRatio;
    for (int i = 0; i < pixelCount; i++)
    {
        this->pixels[i] = img.pixels[i];
    }
}

// Copy int to float
ImageF::ImageF(const ImageI& img)
{
    this->pixels = new pixel4f_t[img.width * img.height];
    this->width = img.width;
    this->height = img.height;
    this->pixelCount = img.pixelCount;
    this->aspectRatio = img.aspectRatio;
    float scalar = 1.0f / 255.0f;
    for (int i = 0; i < pixelCount; i++)
    {
        this->pixels[i].col.r = scalar * float(img.pixels[i].col.r);
        this->pixels[i].col.g = scalar * float(img.pixels[i].col.g);
        this->pixels[i].col.b = scalar * float(img.pixels[i].col.b);
        this->pixels[i].col.a = scalar * float(img.pixels[i].col.a);
    }
}

ImageF::~ImageF()
{
    if (pixels != NULL)
    {
        delete(pixels);
    }
}

void ImageI::write(const char *filename)
{
    stbi_write_png(filename, this->width, this->height, DESIRED_CHANNELS, this->pixels, this->width * sizeof(uint8_t) * 4);
}

void ImageF::toGreyscale(float rScalar, float gScalar, float bScalar)
{
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
    float avg;
    for (int i = 0; i < pixelCount; i++)
    {
        pixels[i].col.r = 1.0 - pixels[i].col.r;
        pixels[i].col.g = 1.0 - pixels[i].col.g;
        pixels[i].col.b = 1.0 - pixels[i].col.b;
    }
}

