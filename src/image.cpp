/********************************************
 * Author: Kyle Bueche
 * File: image.cpp
 *
 *******************************************/

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>
#include <new>
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
    ensureBuffersNull();
}

ImageF::ImageF(const char *filename)
{
    ensureBuffersNull();
    buffer(filename);
}

// Copy constructor
ImageF::ImageF(const ImageF& img)
{
    ensureBuffersNull();
    buffer(img);
}

ImageF::~ImageF()
{
    ensureBuffersDeleted();
}

void ImageF::ensureBuffersNull()
{
    this->pixels = nullptr;
    this->temp = nullptr;
}

void ImageF::ensureBuffersDeleted()
{
    if (this->pixels != nullptr)
        delete(this->pixels);
    if (this->temp != nullptr)
        delete(this->temp);
    this->pixels = nullptr;
    this->temp = nullptr;
    this->width = 0;
    this->height = 0;
    this->pixelCount = 0;
    this->aspectRatio = 1.0f;
}

void ImageF::ensureBufferSize(int width, int height)
{
    if ((this->width != width || this->height != height)
        && (width >= 0 && height >= 0))
    {
        ensureBuffersDeleted();
        this->width = width;
        this->height = height;
        this->pixelCount = width * height;
        this->aspectRatio = width / height;
        pixels = new pixel4f_t[pixelCount];
        temp = new pixel4f_t[pixelCount];
    }
}

bool ImageF::null()
{
    return (pixels == nullptr || temp == nullptr);
}

// From file bufferer
void ImageF::buffer(const char *filename)
{
    int newWidth;
    int newHeight;
    int nrChannels;

    // Desired channels is 4, will always convert to an rgba image.
    unsigned char *data = stbi_load(filename, &newWidth, &newHeight, &nrChannels, NUM_CHANNELS);

    // Handle stbi loading errors
    if (data)
    {
        ensureBufferSize(newWidth, newHeight);
        if (null())
            return;

        int dataIndex = 0;
        float scalar = 1.0f / 255.0f;
        for (int i = 0; i < pixelCount; i++)
        {
            dataIndex = i * 4;
            pixels[i].col.r = scalar * float(data[dataIndex]);
            pixels[i].col.g = scalar * float(data[dataIndex + 1]);
            pixels[i].col.b = scalar * float(data[dataIndex + 2]);
            pixels[i].col.a = scalar * float(data[dataIndex + 3]);
            temp[i].col = pixels[i].col;
        }
        stbi_image_free(data);
    }
    else
    {
        std::cerr << "ERROR: STBI Failed to load the image" << std::endl;
    }
}

// Copy bufferer
void ImageF::buffer(const ImageF& img)
{
    ensureBufferSize(img.width, img.height);
    if (null())
        return;

    for (int i = 0; i < pixelCount; i++)
    {
        this->pixels[i] = img.pixels[i];
    }
}

void ImageF::write(const char *filename)
{
    if (null())
        return;
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

void ImageF::apply()
{
    std::swap<pixel4f_t*>(pixels, temp);
}

void ImageF::toGreyscale(float rScalar, float gScalar, float bScalar)
{
    if (null())
        return;
    float avg;
    for (int i = 0; i < pixelCount; i++)
    {
        avg = rScalar * pixels[i].col.r
            + gScalar * pixels[i].col.g
            + bScalar * pixels[i].col.b;
        temp[i].col.r = avg;
        temp[i].col.g = avg;
        temp[i].col.b = avg;
    }
}

void ImageF::toNegative()
{
    if (null())
        return;
    for (int i = 0; i < pixelCount; i++)
    {
        temp[i].col.r = 1.0 - pixels[i].col.r;
        temp[i].col.g = 1.0 - pixels[i].col.g;
        temp[i].col.b = 1.0 - pixels[i].col.b;
    }
}
