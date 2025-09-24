/********************************************
 * Author: Kyle Bueche
 * File: image.cpp
 *
 *******************************************/

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>
#include <cmath>
#include <new>
#include "image.h"

/************************************************
 * This is kind of terrible but its okay.
 *
 * The full image is copied, this would be meh
 * for loading a large sequence of images.
 *
 ***********************************************/

/************************************************************************
* Author: Kyle Bueche
* RGBA to HSVA conversion
*
* Algorithm adapted from rapidtables.com/convert/color/rgb-to-hsv.html
*
************************************************************************/
pixel4f_hsv_t pixelRGBAtoHSVA(const pixel4f_t& pixel)
{
    pixel4f_hsv_t pixelOut;

    // Precomputation
    float c_max = std::max(std::max(pixel.col.r, pixel.col.g), pixel.col.b);
    float c_min = std::min(std::min(pixel.col.r, pixel.col.g), pixel.col.b);
    float delta = c_max - c_min;

    // Hue calculation
    if (delta == 0.0f)
    {
        pixelOut.col.h = 0.0f;
    }
    else if (c_max == pixel.col.r)
    {
        pixelOut.col.h = 60.0f * std::fmod(((pixel.col.g - pixel.col.b) / delta), 6.0f);
    }
    else if (c_max == pixel.col.g)
    {
        pixelOut.col.h = 60.0f * (((pixel.col.b - pixel.col.r) / delta) + 2.0f);
    }
    else if (c_max == pixel.col.b)
    {
        pixelOut.col.h = 60.0f * (((pixel.col.r - pixel.col.g) / delta) + 4.0f);
    }

    // Saturation calculation
    if (c_max == 0.0f)
    {
        pixelOut.col.s = 0.0f;
    }
    else
    {
        pixelOut.col.s = delta / c_max;
    }

    // Value calculation
    pixelOut.col.v = c_max;
    // Alpha
    pixelOut.col.a = pixel.col.a;
    
    return pixelOut;
}

/************************************************************************
* Author: Kyle Bueche
* HSVA to RGBA conversion
*
* Algorithm adapted from rapidtables.com/convert/color/hsv-to-rgb.html
*
************************************************************************/
pixel4f_t pixelHSVAtoRGBA(const pixel4f_hsv_t& pixel)
{
    pixel4f_t pixelOut;
    // Ensure 0 <= H < 360,
    //        0 <= S <= 1,
    //        0 <= V <= 1
    float H = std::min(std::max(pixel.col.h, 0.0f), 359.99f);
    float S = std::min(std::max(pixel.col.s, 0.0f), 1.0f);
    float V = std::min(std::max(pixel.col.v, 0.0f), 1.0f);

    // Precomputation
    float C = V * S;
    float X = C * (1.0f - std::abs(std::fmod((H / 60.0f), 2.0f) - 1));
    float m = V - C;

    // RGB Calculation
    if (0 <= H < 60.0f)
    {
        pixelOut.col.r = C;
        pixelOut.col.g = X;
        pixelOut.col.b = 0.0f;
    }
    else if (60.0f <= H < 120.0f)
    {
        pixelOut.col.r = X;
        pixelOut.col.g = C;
        pixelOut.col.b = 0.0f;
    }
    else if (120.0f <= H < 180.0f)
    {
        pixelOut.col.r = 0.0f;
        pixelOut.col.g = C;
        pixelOut.col.b = X;
    }
    else if (180.0f <= H < 240.0f)
    {
        pixelOut.col.r = 0.0f;
        pixelOut.col.g = X;
        pixelOut.col.b = C;
    }
    else if (240.0f <= H < 300.0f)
    {
        pixelOut.col.r = X;
        pixelOut.col.g = 0.0f;
        pixelOut.col.b = C;
    }
    else if (300.0f <= H < 360.0f)
    {
        pixelOut.col.r = C;
        pixelOut.col.g = 0.0f;
        pixelOut.col.b = X;
    }
    pixelOut.col.r += m;
    pixelOut.col.g += m;
    pixelOut.col.b += m;

    // Alpha
    pixelOut.col.a = pixel.col.a;

    return pixelOut;
}

ImageF::ImageF()
{
    ensureBuffersNull();
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
        this->aspectRatio = float(width) / float(height);
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
        intBuffer[i].col.r = std::min(uint8_t(255.99f * pixels[i].col.r), uint8_t(255));
        intBuffer[i].col.g = std::min(uint8_t(255.99f * pixels[i].col.g), uint8_t(255));
        intBuffer[i].col.b = std::min(uint8_t(255.99f * pixels[i].col.b), uint8_t(255));
        intBuffer[i].col.a = std::min(uint8_t(255.99f * pixels[i].col.a), uint8_t(255));
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
    {
        std::cout << "nullptrIMG" << std::endl;
        return;
    }
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

void ImageF::threshold(float thresh)
{
    if (null())
        return;
    for (int i = 0; i < pixelCount; i++)
    {
        float avg = (pixels[i].col.r + pixels[i].col.g + pixels[i].col.b) / 3.0f;
        if (avg > thresh)
        {
            temp[i].col.r = 1.0;
            temp[i].col.g = 1.0;
            temp[i].col.b = 1.0;
        }
        else
        {
            temp[i].col.r = 0.0f;
            temp[i].col.g = 0.0f;
            temp[i].col.b = 0.0f;
        }
    }
}

void ImageF::scaleContrast(float val)
{
    if (null())
        return;
    for (int i = 0; i < pixelCount; i++)
    {
         temp[i].col.r = pixels[i].col.r * val;
         temp[i].col.g = pixels[i].col.g * val;
         temp[i].col.b = pixels[i].col.b * val;
    }
}

void ImageF::colorTint(float r, float g, float b, float a)
{
    if (null())
        return;
    for (int i = 0; i < pixelCount; i++)
    {
         temp[i].col.r = pixels[i].col.r * (1.0f - a) + r * a;
         temp[i].col.g = pixels[i].col.g * (1.0f - a) + g * a;
         temp[i].col.b = pixels[i].col.b * (1.0f - a) + b * a;
    }
}

void ImageF::adjustHSV(float h, float s, float v)
{
    if (null())
        return;
    for (int i = 0; i < pixelCount; i++)
    {
        pixel4f_hsv_t pixel = pixelRGBAtoHSVA(pixels[i]);
        pixel.col.h = pixel.col.h + h;
        pixel.col.s = pixel.col.s * s;
        pixel.col.v = pixel.col.v * v;
        temp[i] = pixelHSVAtoRGBA(pixel);
    }
}

void ImageF::rotateHue(float val)
{
    if (null())
        return;
    for (int i = 0; i < pixelCount; i++)
    {
        pixel4f_hsv_t pixel = pixelRGBAtoHSVA(pixels[i]);
        pixel.col.h = pixel.col.h + val;
        temp[i] = pixelHSVAtoRGBA(pixel);
    }
}

void ImageF::scaleSaturation(float val)
{
    if (null())
        return;
    for (int i = 0; i < pixelCount; i++)
    {
        pixel4f_hsv_t pixel = pixelRGBAtoHSVA(pixels[i]);
        pixel.col.s = pixel.col.s * val;
        temp[i] = pixelHSVAtoRGBA(pixel);
    }
}
        
void ImageF::scaleValue(float val)
{
    if (null())
        return;
    for (int i = 0; i < pixelCount; i++)
    {
        pixel4f_hsv_t pixel = pixelRGBAtoHSVA(pixels[i]);
        pixel.col.v = pixel.col.v * val;
        temp[i] = pixelHSVAtoRGBA(pixel);
    }
}
