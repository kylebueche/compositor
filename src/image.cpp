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

ImagePipeline::ImagePipeline()
{
    this->input = nullptr;
    this->temp1 = nullptr;
    this->temp2 = nullptr;
    this->temp3 = nullptr;
    this->output = nullptr;
    this->width = 0;
    this->height = 0;
    this->aspectRatio = 1.0f;
    this->pixelCount = 0;
    this->bufferSize = 0;
}

ImagePipeline::~ImagePipeline()
{
    ensureBuffersDeleted();
}

void ImagePipeline::ensureBuffersDeleted()
{
    if (this->input != nullptr)
        delete(this->pixels);
    this->input = nullptr;
    this->temp1 = nullptr;
    this->temp2 = nullptr;
    this->temp3 = nullptr;
    this->output = nullptr;
    this->width = 0;
    this->height = 0;
    this->aspectRatio = 1.0f;
    this->pixelCount = 0;
    this->bufferSize = 0;
}

void ImagePipeline::ensureBufferSize(int width, int height)
{
    if (width >= 0 && height >= 0)
    {
        int newPixelCount = width * height;
        if (newPixelCount > this->bufferSize)
        {
            ensureBuffersDeleted();
            this->input = new pixel4f_t[newPixelCount * 5];
            this->temp1 = &(this->input[1 * newPixelCount]);
            this->temp2 = &(this->input[2 * newPixelCount]);
            this->temp3 = &(this->input[3 * newPixelCount]);
            this->output = &(this->input[4 * newPixelCount]);
            this->bufferSize = newPixelCount;
        }
        if (null())
            return;

        this->width = width;
        this->height = height;
        this->pixelCount = newPixelCount;
        this->aspectRatio = float(width) / float(height);
    }
}

const bool ImagePipeline::null() const
{
    bool anyNull = (input == nullptr || temp1 == nullptr || temp2 == nullptr || temp3 == nullptr || output == nullptr);
    if (anyNull)
    {
        std::cerr("Image buffer loading failed.");
    }
    return anyNull;
}

// From file bufferer
void ImagePipeline::buffer(const char *filename)
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
        pixel4i_t* intBuffer = (pixel4i_t*) data;
        for (int i = 0; i < pixelCount; i++)
        {
            input[i] = pixelItoF(intBuffer[i]);
            output[i] = input[i];
        }
        stbi_image_free(data);
    }
    else
    {
        std::cerr << "ERROR: STBI Failed to load the image" << std::endl;
    }
}

// TODO: Handle intBuffer allocation failure
void ImagePipeline::write(const char *filename)
{
    pixel4i_t *intBuffer = new pixel4i_t[pixelCount];
    for (int i = 0; i < pixelCount; i++)
    {
        intBuffer[i] = pixelFtoI(output[i]);
    }
    stbi_write_png(filename, this->width, this->height, NUM_CHANNELS, intBuffer, this->width * sizeof(uint8_t) * NUM_CHANNELS);
    delete(intBuffer);
}

void ImagePipeline::toGreyscale(pixel4f_t weights)
{
    float avg;
    for (int i = 0; i < pixelCount; i++)
    {
        avg = rScalar * input[i].r
            + gScalar * input[i].g
            + bScalar * input[i].b;
        output[i].r = avg;
        output[i].g = avg;
        output[i].b = avg;
    }
}

void ImagePipeline::toNegative()
{
    for (int i = 0; i < pixelCount; i++)
    {
        output[i].r = 1.0 - input[i].r;
        output[i].g = 1.0 - input[i].g;
        output[i].b = 1.0 - input[i].b;
    }
}

void ImagePipeline::threshold(float threshold)
{
    for (int i = 0; i < pixelCount; i++)
    {
        float avg = (input[i].r + input[i].g + input[i].b) / 3.0f;
        if (avg > threshold)
        {
            output[i].r = 1.0f;
            output[i].g = 1.0f;
            output[i].b = 1.0f;
            output[i].a = 1.0f;
        }
        else
        {
            output[i].r = 0.0f;
            output[i].g = 0.0f;
            output[i].b = 0.0f;
            output[i].a = 0.0f;
        }
    }
}

void ImagePipeline::thresholdColor(float thresh, float strength)
{
    for (int i = 0; i < pixelCount; i++)
    {
        float avg = (input[i].r + input[i].g + input[i].b) / 3.0f;
        if (avg > thresh)
        {
            output[i].r = input[i].r * strength;
            output[i].g = input[i].g * strength;
            output[i].b = input[i].b * strength;
            output[i].a = input[i].a * strength;
        }
        else
        {
            output[i].r = 0.0f;
            output[i].g = 0.0f;
            output[i].b = 0.0f;
            output[i].a = 0.0f;
        }
    }
}

pixel4f_t ImagePipeline::max()
{
    pixel4f_t max;
    max.r = - std::numeric_limits<float>::infinity();
    max.g = - std::numeric_limits<float>::infinity();
    max.b = - std::numeric_limits<float>::infinity();
    max.a = - std::numeric_limits<float>::infinity();

    for (int i = 0; i < pixelCount; i++)
    {
        max.r = std::max(input[i].r, max.r);
        max.g = std::max(input[i].g, max.g);
        max.b = std::max(input[i].b, max.b);
        max.a = std::max(input[i].a, max.a);
    }

    return max;
}

pixel4f_t ImagePipeline::min()
{
    pixel4f_t min;
    min.r = std::numeric_limits<float>::infinity();
    min.g = std::numeric_limits<float>::infinity();
    min.b = std::numeric_limits<float>::infinity();
    min.a = std::numeric_limits<float>::infinity();

    for (int i = 0; i < pixelCount; i++)
    {
        min.r = std::min(input[i].r, min.r);
        min.g = std::min(input[i].g, min.g);
        min.b = std::min(input[i].b, min.b);
        min.a = std::min(input[i].a, min.a);
    }

    return min;
}

void ImagePipeline::scaleContrast(float contrast)
{
    float higherBound = contrast;
    float lowerBound = 1.0f / contrast;
    float deltaOut = higherBound - lowerBound;
    pixel4f_t minPixel = min();
    pixel4f_t maxPixel = max();
    float min = std::min(std::min(minPixel.r, minPixel.g), minPixel.b);
    float max = std::max(std::max(maxPixel.r, maxPixel.g), maxPixel.b);
    float delta = max - min;
    
    for (int i = 0; i < pixelCount; i++)
    {
         output[i].r = (((input[i].r - min) / delta) * deltaOut) + lowerBound;
         output[i].g = (((input[i].g - min) / delta) * deltaOut) + lowerBound;
         output[i].b = (((input[i].b - min) / delta) * deltaOut) + lowerBound;
    }
}

void ImagePipeline::colorTint(pixel4f_t tint)
{
    for (int i = 0; i < pixelCount; i++)
    {
         output[i].r = input[i].r * (1.0f - tint.a) + tint.r * tint.a;
         output[i].g = input[i].g * (1.0f - tint.a) + tint.g * tint.a;
         output[i].b = input[i].b * (1.0f - tint.a) + tint.b * tint.a;
    }
}

void ImagePipeline::adjustHSV(pixel4f_hsv_t hsv)
{
    if (null())
        return;
    for (int i = 0; i < pixelCount; i++)
    {
        pixel4f_hsv_t pixel = pixelRGBAtoHSVA(input[i]);
        pixel.h = pixel.h + hsv.h;
        pixel.s = pixel.s * hsv.s;
        pixel.v = pixel.v * hsv.v;
        output[i] = pixelHSVAtoRGBA(pixel);
    }
}

void ImagePipeline::gaussianBlur(int kernel)
{
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
            pixelIndex = index(x, y, width, height);
            temp1[pixelIndex].r = 0.0f;
            temp1[pixelIndex].g = 0.0f;
            temp1[pixelIndex].b = 0.0f;
            for (int i = -offset; i <= +offset; i++)
            {
                convolutionIndex = index(x + i, y, width, height);
                temp1[pixelIndex].r += convolutionVector[std::abs(i)] * input[convolutionIndex].r;
                temp1[pixelIndex].g += convolutionVector[std::abs(i)] * input[convolutionIndex].g;
                temp1[pixelIndex].b += convolutionVector[std::abs(i)] * input[convolutionIndex].b;
            }
        }
    }
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            pixelIndex = index(x, y, width, height);
            output[pixelIndex].r = 0.0f;
            output[pixelIndex].g = 0.0f;
            output[pixelIndex].b = 0.0f;
            for (int j = -offset; j <= +offset; j++)
            {
                convolutionIndex = index(x, y + j, width, height);
                output[pixelIndex].r += convolutionVector[std::abs(j)] * temp1[convolutionIndex].r;
                output[pixelIndex].g += convolutionVector[std::abs(j)] * temp1[convolutionIndex].g;
                output[pixelIndex].b += convolutionVector[std::abs(j)] * temp1[convolutionIndex].b;
            }
        }
    }
}

void ImagePipeline::blendForeground(pixel4f_t* fg, pixel4f_t* bg)
{
    for (int i = 0; i < pixelCount; i++)
    {
        float new_a = fg[i].a + bf[i].a * (1.0f - fg[i].a);
        if (new_a == 0.0f)
        {
            output[i].r = 0.0f;
            output[i].g = 0.0f;
            output[i].b = 0.0f;
            output[i].a = 0.0f;
        }
        else
        {
            output[i].r = (fg[i].r * fg[i].a + bg[i].r * bg[i].a * (1.0f - fg[i].a)) / new_a;
            output[i].g = (fg[i].g * fg[i].a + bg[i].g * bg[i].a * (1.0f - fg[i].a)) / new_a;
            output[i].b = (fg[i].b * fg[i].a + bg[i].b * bg[i].a * (1.0f - fg[i].a)) / new_a;
            output[i].a = new_a;
        }
    }
}

// TODO: Fix alpha?
void ImagePipeline::add(pixel4f_t* fg, pixel4f_t* bg)
{
    for (int i = 0; i < pixelCount; i++)
    {
        output[i].r = fg[i].r + bg[i].r;
        output[i].g = fg[i].g + bg[i].r;
        output[i].b = fg[i].b + bg[i].r;
        output[i].a = fg[i].a;
    }
}

void ImagePipeline::bloom(float threshold, int kernel)
{
    threshold(threshold);
    std::swap<pixel4f_t*>(temp2, output);
    gaussianBlur(kernel);
    add(output, temp2);
}
