/********************************************
 * Author: Kyle Bueche
 * File: image.h
 *
 *******************************************/

#ifndef IMAGE_H
#define IMAGE_H

#include <cstring>
#include <cstdint>
#include <iostream>

const int NUM_CHANNELS = 4;

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} pixel4i_t;


typedef struct
{
    float r;
    float g;
    float b;
    float a;
} pixel4f_t;

typedef struct
{
    float h;
    float s;
    float v;
    float a;
} pixel4f_hsv_t;

inline int clamp(int value, int min, int max);
inline int index(int x, int y, int width, int height);
inline pixel4f_hsv_t pixelRGBAtoHSVA(const pixel4f_t& pixel);
inline pixel4f_t pixelHSVAtoRGBA(const pixel4f_hsv_t& pixel);

class ImagePipeline
{
public:
    int width;
    int height;
    float aspectRatio;

    int pixelCount;
    int bufferSize;

    pixel4f_t *input;
    pixel4f_t *temp1;
    pixel4f_t *temp2;
    pixel4f_t *temp3;
    pixel4f_t *output;

    ImagePipeline(int width, int height);
    ~ImagePipeline();
    const bool null() const;
    void ensureBuffersDeleted();
    void ensureBufferSize(int width, int height);
    void buffer(const char* filename); // Load image from file
    void write(const char* filename); // Write image to file

    void toGreyscale(pixel4f_t weights);
    void toNegative();
    pixel4f_t max();
    pixel4f_t min();
    void scaleContrast(float contrast);
    void colorTint(pixel4f_t tint);
    void threshold(float threshold);
    void thresholdColor(float threshold, float strength);
    void adjustHSV(pixel4f_hsv_t hsv);
    void gaussianBlur(int kernel);
    void blendForeground(pixel4f_t* a, pixel4f_t* b);
    void add(pixel4f_t* a, pixel4f_t* b);
    void bloom(float threshold, int kernel);
};

/************************************************************************
* Author: Kyle Bueche
* Branchless clamp
*
************************************************************************/

inline int clamp(int val, int min, int max)
{
    const int t = (val < min) ? min : val;
    return (t > max) ? max : t;
}

inline float clamp(float val, float min, float max)
{
    const float t = (val < min) ? min : val;
    return (t > max) ? max : t;
}

/************************************************************************
* Author: Kyle Bueche
* Nearest in-bounds index
*
************************************************************************/
inline int index(int x, int y, int width, int height)
{
    return clamp(y, 0, height - 1) * width + clamp(x, 0, width - 1);
}

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

#endif
