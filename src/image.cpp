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
#include <numbers>
#include <vector>
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
* Algorithm adapted from rapidtables.com/convertor/rgb-to-hsv.html
*
************************************************************************/
pixel4f_hsv_t pixelRGBAtoHSVA(const pixel4f_t& pixel)
{
    pixel4f_hsv_t pixelOut;

    // Precomputation
    float c_max = std::max(std::max(pixel.r, pixel.g), pixel.b);
    float c_min = std::min(std::min(pixel.r, pixel.g), pixel.b);
    float delta = c_max - c_min;

    if (delta > 0.0f)
    {
        // Hue calculation
        if (c_max == pixel.r)
        {
            pixelOut.h = 60.0f * std::fmod(((pixel.g - pixel.b) / delta), 6.0f);
        }
        else if (c_max == pixel.g)
        {
            pixelOut.h = 60.0f * (((pixel.b - pixel.r) / delta) + 2.0f);
        }
        else if (c_max == pixel.b)
        {
            pixelOut.h = 60.0f * (((pixel.r - pixel.g) / delta) + 4.0f);
        }

        // Saturation calculation
        if (c_max > 0.0f)
        {
            pixelOut.s = delta / c_max;
        }
        else
        {
            pixelOut.s = 0.0f;
        }
    }

    else
    {
        pixelOut.h = 0.0f;
        pixelOut.s = 0.0f;
    }

    // Value calculation
    pixelOut.v = c_max;
    // Alpha
    pixelOut.a = pixel.a;

    if (pixelOut.h < 0)
    {
        pixelOut.h = 360.0f + pixelOut.h;
    }
    
    return pixelOut;
}

/************************************************************************
* Author: Kyle Bueche
* HSVA to RGBA conversion
*
* Algorithm adapted from rapidtables.com/convertor/hsv-to-rgb.html
*
************************************************************************/
pixel4f_t pixelHSVAtoRGBA(const pixel4f_hsv_t& pixel)
{
    pixel4f_t pixelOut;
    // Ensure 0 <= H < 360,
    //        0 <= S <= 1,
    //        0 <= V <= 1
    float H = std::fmod(pixel.h, 360.0f);
    if (H < 0.0f)
    {
        H = H + 360.0f;
    }
    float S = std::min(std::max(pixel.s, 0.0f), 1.0f);
    float V = std::min(std::max(pixel.v, 0.0f), 1.0f);

    // Precomputation
    float C = V * S;
    float X = C * (1 - std::abs(std::fmod(H / 60.0f, 2) - 1));
    float m = V - C;

    // RGB Calculation
    if (0.0f <= H && H < 60.0f)
    {
        pixelOut.r = C;
        pixelOut.g = X;
        pixelOut.b = 0.0f;
    }
    else if (60.0f <= H && H < 120.0f)
    {
        pixelOut.r = X;
        pixelOut.g = C;
        pixelOut.b = 0.0f;
    }
    else if (120.0f <= H && H < 180.0f)
    {
        pixelOut.r = 0.0f;
        pixelOut.g = C;
        pixelOut.b = X;
    }
    else if (180.0f <= H && H < 240.0f)
    {
        pixelOut.r = 0.0f;
        pixelOut.g = X;
        pixelOut.b = C;
    }
    else if (240.0f <= H && H < 300.0f)
    {
        pixelOut.r = X;
        pixelOut.g = 0.0f;
        pixelOut.b = C;
    }
    else if (300.0f <= H && H < 360.0f)
    {
        pixelOut.r = C;
        pixelOut.g = 0.0f;
        pixelOut.b = X;
    }
    else
    {
        pixelOut.r = 0.0f;
        pixelOut.g = 0.0f;
        pixelOut.b = 0.0f;
    }

    pixelOut.r += m;
    pixelOut.g += m;
    pixelOut.b += m;

    // Alpha
    pixelOut.a = pixel.a;

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
    this->output = nullptr;
}

void ImageF::ensureBuffersDeleted()
{
    if (this->pixels != nullptr)
        delete(this->pixels);
    if (this->temp != nullptr)
        delete(this->temp);
    if (this->output != nullptr)
        delete(this->output);
    this->pixels = nullptr;
    this->temp = nullptr;
    this->output = nullptr;
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
        output = new pixel4f_t[pixelCount];
    }
}

const bool ImageF::null() const
{
    return (pixels == nullptr || temp == nullptr || output == nullptr);
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
            pixels[i].r = scalar * float(data[dataIndex]);
            pixels[i].g = scalar * float(data[dataIndex + 1]);
            pixels[i].b = scalar * float(data[dataIndex + 2]);
            pixels[i].a = scalar * float(data[dataIndex + 3]);
            temp[i] = pixels[i];
            
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
        intBuffer[i].r = uint8_t(255.99f * std::max(std::min(temp[i].r, 1.0f), 0.0f));
        intBuffer[i].g = uint8_t(255.99f * std::max(std::min(temp[i].g, 1.0f), 0.0f));
        intBuffer[i].b = uint8_t(255.99f * std::max(std::min(temp[i].b, 1.0f), 0.0f));
        intBuffer[i].a = uint8_t(255.99f * std::max(std::min(temp[i].a, 1.0f), 0.0f));
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
        avg = rScalar * pixels[i].r
            + gScalar * pixels[i].g
            + bScalar * pixels[i].b;
        temp[i].r = avg;
        temp[i].g = avg;
        temp[i].b = avg;
    }
}

void ImageF::toNegative()
{
    if (null())
        return;
    for (int i = 0; i < pixelCount; i++)
    {
        temp[i].r = 1.0 - pixels[i].r;
        temp[i].g = 1.0 - pixels[i].g;
        temp[i].b = 1.0 - pixels[i].b;
    }
}

void ImageF::threshold(float thresh)
{
    if (null())
        return;
    for (int i = 0; i < pixelCount; i++)
    {
        float avg = (pixels[i].r + pixels[i].g + pixels[i].b) / 3.0f;
        if (avg > thresh)
        {
            temp[i].r = 1.0f;
            temp[i].g = 1.0f;
            temp[i].b = 1.0f;
            temp[i].a = 1.0f;
        }
        else
        {
            temp[i].r = 0.0f;
            temp[i].g = 0.0f;
            temp[i].b = 0.0f;
            temp[i].a = 0.0f;
        }
    }
}

void ImageF::thresholdColor(float thresh, float strength)
{
    if (null())
        return;
    for (int i = 0; i < pixelCount; i++)
    {
        float avg = (pixels[i].r + pixels[i].g + pixels[i].b) / 3.0f;
        if (avg > thresh)
        {
            temp[i].r = pixels[i].r * strength;
            temp[i].g = pixels[i].g * strength;
            temp[i].b = pixels[i].b * strength;
            temp[i].a = pixels[i].a * strength;
        }
        else
        {
            temp[i].r = 0.0f;
            temp[i].g = 0.0f;
            temp[i].b = 0.0f;
            temp[i].a = 0.0f;
        }
    }
}

pixel4f_t ImageF::max()
{
    pixel4f_t max;
    max.r = - std::numeric_limits<float>::infinity();
    max.g = - std::numeric_limits<float>::infinity();
    max.b = - std::numeric_limits<float>::infinity();
    max.a = - std::numeric_limits<float>::infinity();
    if (null())
        return max;

    for (int i = 0; i < pixelCount; i++)
    {
        max.r = std::max(pixels[i].r, max.r);
        max.g = std::max(pixels[i].g, max.g);
        max.b = std::max(pixels[i].b, max.b);
        max.a = std::max(pixels[i].a, max.a);
    }

    return max;
}

pixel4f_t ImageF::min()
{
    pixel4f_t min;
    min.r = std::numeric_limits<float>::infinity();
    min.g = std::numeric_limits<float>::infinity();
    min.b = std::numeric_limits<float>::infinity();
    min.a = std::numeric_limits<float>::infinity();
    if (null())
        return min;

    for (int i = 0; i < pixelCount; i++)
    {
        min.r = std::min(pixels[i].r, min.r);
        min.g = std::min(pixels[i].g, min.g);
        min.b = std::min(pixels[i].b, min.b);
        min.a = std::min(pixels[i].a, min.a);
    }

    return min;
}

void ImageF::scaleContrast(float lowerBound, float higherBound)
{
    if (null())
        return;

    float deltaOut = higherBound - lowerBound;
    pixel4f_t minPixel = min();
    pixel4f_t maxPixel = max();
    float min = std::min(std::min(minPixel.r, minPixel.g), minPixel.b);
    float max = std::max(std::max(maxPixel.r, maxPixel.g), maxPixel.b);
    float delta = max - min;
    
    for (int i = 0; i < pixelCount; i++)
    {
         temp[i].r = (((pixels[i].r - min) / delta) * deltaOut) + lowerBound;
         temp[i].g = (((pixels[i].g - min) / delta) * deltaOut) + lowerBound;
         temp[i].b = (((pixels[i].b - min) / delta) * deltaOut) + lowerBound;
    }
}

void ImageF::colorTint(float r, float g, float b, float a)
{
    if (null())
        return;
    for (int i = 0; i < pixelCount; i++)
    {
         temp[i].r = pixels[i].r * (1.0f - a) + r * a;
         temp[i].g = pixels[i].g * (1.0f - a) + g * a;
         temp[i].b = pixels[i].b * (1.0f - a) + b * a;
    }
}

void ImageF::adjustHSV(float h, float s, float v)
{
    if (null())
        return;
    for (int i = 0; i < pixelCount; i++)
    {
        pixel4f_hsv_t pixel = pixelRGBAtoHSVA(pixels[i]);
        pixel.h = pixel.h + h;
        pixel.s = pixel.s * s;
        pixel.v = pixel.v * v;
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
        pixel.h = pixel.h + val;
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
        pixel.s = pixel.s * val;
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
        pixel.v = pixel.v * val;
        temp[i] = pixelHSVAtoRGBA(pixel);
    }
}

void ImageF::gaussianBlur(int kernel)
{
    if (null())
    {
        return;
    }

    if (kernel % 2 == 0)
    {
        kernel++;
    }
    int offset = int(kernel / 2);
    // Just make one corner of the kernel
    std::vector<float> convolutionVector(offset + 1, 0);
    float stdev = float(kernel - 1) / 6.0f;
    float one_over_sqrt_2_pi_stdevsqrd = 1.0f/sqrt(2.0f * std::numbers::pi * stdev * stdev);
    for (int x = 0; x <= +offset; x++)
    {
        convolutionVector[x] = one_over_sqrt_2_pi_stdevsqrd * exp(- (x * x) / (2.0f * stdev * stdev));
    }
    int pixelIndex = 0;
    int convolutionIndex = 0;
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            pixelIndex = index(x, y);
            output[pixelIndex].r = 0.0f;
            output[pixelIndex].g = 0.0f;
            output[pixelIndex].b = 0.0f;
            for (int i = -offset; i <= +offset; i++)
            {
                convolutionIndex = index(x + i, y);
                output[pixelIndex].r += convolutionVector[std::abs(i)] * pixels[convolutionIndex].r;
                output[pixelIndex].g += convolutionVector[std::abs(i)] * pixels[convolutionIndex].g;
                output[pixelIndex].b += convolutionVector[std::abs(i)] * pixels[convolutionIndex].b;
            }
        }
    }
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            pixelIndex = index(x, y);
            temp[pixelIndex].r = 0.0f;
            temp[pixelIndex].g = 0.0f;
            temp[pixelIndex].b = 0.0f;
            for (int j = -offset; j <= +offset; j++)
            {
                convolutionIndex = index(x, y + j);
                temp[pixelIndex].r += convolutionVector[std::abs(j)] * output[convolutionIndex].r;
                temp[pixelIndex].g += convolutionVector[std::abs(j)] * output[convolutionIndex].g;
                temp[pixelIndex].b += convolutionVector[std::abs(j)] * output[convolutionIndex].b;
            }
        }
    }
}

void ImageF::blendForeground(const ImageF& fg)
{
    if (null() || fg.null() || pixelCount != fg.pixelCount)
        return;
    for (int i = 0; i < pixelCount; i++)
    {
        float new_a = fg.pixels[i].a + pixels[i].a * (1.0f - fg.pixels[i].a);
        if (new_a == 0.0f)
        {
            temp[i].r = 0.0f;
            temp[i].g = 0.0f;
            temp[i].b = 0.0f;
            temp[i].a = 0.0f;
        }
        else
        {
            temp[i].r = (fg.pixels[i].r * fg.pixels[i].a + pixels[i].r * pixels[i].a * (1.0f - fg.pixels[i].a)) / new_a;
            temp[i].g = (fg.pixels[i].g * fg.pixels[i].a + pixels[i].g * pixels[i].a * (1.0f - fg.pixels[i].a)) / new_a;
            temp[i].b = (fg.pixels[i].b * fg.pixels[i].a + pixels[i].b * pixels[i].a * (1.0f - fg.pixels[i].a)) / new_a;
            temp[i].a = new_a;
        }
    }
}

void ImageF::add(const ImageF& img)
{
    if (null() || img.null() || pixelCount != img.pixelCount)
        return;
    for (int i = 0; i < pixelCount; i++)
    {
        temp[i].r = pixels[i].r + img.pixels[i].r;
        temp[i].g = pixels[i].g + img.pixels[i].r;
        temp[i].b = pixels[i].b + img.pixels[i].r;
        temp[i].a = pixels[i].a;
    }
}

void ImageF::bloom(float threshold, int kernel)
{
    if (null())
        return;
    static ImageF foreground;
    foreground.buffer(*this);
    foreground.threshold(threshold);
    foreground.apply();
    foreground.gaussianBlur(kernel);
    foreground.apply();
    this->add(foreground);
}
