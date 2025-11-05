/************************************************************************
 * Author: Kyle Bueche
 * File: image.h
 *
 * Header with some inline functions for RGBA and HSVA operations.
 * Pixel functions are used in a greater ImagePipeline class.
************************************************************************/

#ifndef IMAGE_H
#define IMAGE_H

#include <cstring>
#include <cstdint>
#include <iostream>
#include <cmath>
#include <numbers>

const int NUM_CHANNELS = 4;

struct pixel4i_t
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct pixel4f_t
{
    float r;
    float g;
    float b;
    float a;
};

struct pixel4f_hsv_t
{
    float h;
    float s;
    float v;
    float a;
};

struct vec2
{
    float x;
    float y;
};

struct vec2_quad_t
{
    vec2 topLeft;
    vec2 topRight;
    vec2 bottomLeft;
    vec2 bottomRight;
};

struct rgba_quad_t
{
    pixel4f_t topLeft;
    pixel4f_t topRight;
    pixel4f_t bottomLeft;
    pixel4f_t bottomRight;
};


// Standard pixel operations
inline pixel4f_t operator+(const pixel4f_t& fg, const pixel4f_t& bg);
inline pixel4f_t operator-(const pixel4f_t& fg, const pixel4f_t& bg);
inline pixel4f_t operator*(const float& scalar, const pixel4f_t& pixel);
inline pixel4f_t operator*(const pixel4f_t& pixel, const float& scalar);
inline pixel4f_t operator/(const pixel4f_t& pixel, const float& scalar);
inline pixel4f_t operator*(const pixel4f_t& pixel1, const pixel4f_t& pixel2);
inline pixel4f_t negate(const pixel4f_t& pixel1, const pixel4f_t& pixel2);

inline int clamp(int value, int min, int max);
inline int index(int x, int y, int width, int height);
inline pixel4f_hsv_t pixelRGBAtoHSVA(const pixel4f_t& pixel);
inline pixel4f_t pixelHSVAtoRGBA(const pixel4f_hsv_t& pixel);
inline pixel4f_t pixelItoF(const pixel4i_t& pixel);
inline pixel4i_t pixelFtoI(const pixel4f_t& pixel);

// Handles file I/O, dynamic sizing, single-image storing.
class Image
{
public:
    struct pixel4f_t* buffer;
    int width;
    int height;
    float aspectRatio;
    
    int pixelCount; // Current image size
    int bufferSize; // Underlying memory size
    Image();
    ~Image();
    const bool null() const; // Check if buffer is nullptr
    void ensureBufferDeleted();
    void ensureBufferSize(int width, int height);
    void read(const char* filename); // Load image from file
    void read(const Image& image); // Copy image from other image
    void write(const char* filename); // Write image to file

    // Make myImage(i) valid:
    inline pixel4f_t& operator[](size_t i) noexcept {
        return buffer[i];
    }
    inline const pixel4f_t& operator[](size_t i) const noexcept {
        return buffer[i];
    }

    // Make myImage(x, y) valid
    inline pixel4f_t& operator()(size_t x, size_t y) noexcept {
        return buffer[y * width + x];
    }
    inline const pixel4f_t& operator()(size_t x, size_t y) const noexcept {
        return buffer[y * width + x];
    }

    // Nearest in-bounds 2D index, flattened into 1D
    inline pixel4f_t& clamped(size_t x, size_t y) noexcept {
        return buffer[clamp(y, 0, height - 1) * width + clamp(x, 0, width - 1)];
    }
    inline const pixel4f_t& clamped(size_t x, size_t y) const noexcept {
        return buffer[clamp(y, 0, height - 1) * width + clamp(x, 0, width - 1)];
    }
};

// Handles operations that require a memory pool.
class ImagePipeline
{
public:
    // Workspace
    Image temp1;
    Image temp2;
    Image temp3;

    // 1 Image input, non-Image output
    pixel4f_t max(const Image& image);
    pixel4f_t min(const Image& image);

    // 1 Image input, 1 Image output
    // Ensure output fits input size
    void toNegative(const Image& in, Image& out);
    void scaleContrast(const Image& in, Image& out, float contrast);
    void scaleBrightness(const Image& in, Image& out, float brightness);
    void scaleTransparency(const Image& in, Image& out, float transparency);
    void toGreyscale(const Image& in, Image& out, pixel4f_t weights);
    void colorTint(const Image& in, Image& out, pixel4f_t tint);
    void threshold(const Image& in, Image& out, float threshold);
    void thresholdColor(const Image& in, Image& out, float threshold);
    void adjustHSV(const Image& in, Image& out, pixel4f_hsv_t hsv);
    void gaussianBlur(const Image& in, Image& out, int kernel);
    void gaussianDeBlur(const Image& in, Image& out, int kernel);
    void bloom(const Image& in, Image& out, float threshold, int kernel, float strength);

    // 2 Image input, 1 Image output
    // Ensure output fits the larger width and larger height from each image
    void blendForeground(const Image& fg, const Image& bg, Image& out);
    void add(const Image& in1, const Image& in2, Image& out);
    void subtract(const Image& in1, const Image& in2, Image& out);

    // Mask output
    void maskify(const Image& imgIn, Image& maskOut);
    void horizontalMask(Image& maskOut, float t, int feathering, int width, int height);
    void verticalMask(Image& maskOut, float t, int feathering, int width, int height);
    void circleMask(Image& maskOut, float t, int feathering, int width, int height);
    
    // 2 Image input, 1 Mask input, 1 Image output
    // Not size checked for now
    void composite(const Image& imgIn1, const Image& imgIn2, Image& imgOut, const Image& mask);
};

/************************************************************************
* Branchless clamp for ints and floats
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

inline float mylerp(float t, float t0, float t1)
{
    return clamp(t * (t1 - t0) + t0, 0.0f, 1.0f);
}

inline float mylerp(float t, int t0, int t1)
{
    return mylerp(t, float(t0), float(t1));
}

inline vec2 mylerp(float t, vec2 t0, vec2 t1)
{
    return { mylerp(t, t0.x, t1.x), mylerp(t, t0.y, t1.y) };
}

inline pixel4f_t mylerp(float t, pixel4f_t t0, pixel4f_t t1)
{
    return { mylerp(t, t0.r, t1.r),
             mylerp(t, t0.g, t1.g),
             mylerp(t, t0.b, t1.b),
             mylerp(t, t0.a, t1.a) };
}

inline float cubic_interpolation(float t, float t0, float t1)
{
    return 1.0f;
}
    
inline pixel4f_t bilinear_interpolation(vec2 point, vec2 topLeft, rgba_quad_t rgbaQuad)
{
    float t_x = point.x - topLeft.x;
    float t_y = point.y - topLeft.y;

    pixel4f_t topColor = mylerp(t_x, rgbaQuad.topLeft, rgbaQuad.topRight);
    pixel4f_t bottomColor = mylerp(t_x, rgbaQuad.bottomLeft, rgbaQuad.bottomRight);
    pixel4f_t color = mylerp(t_y, bottomColor, topColor);
    return color;
}

/************************************************************************
* RGBA Blend adapted from Wikipedia
* TODO: Clamp is in place as a branchless safeguard against division by 0.
* May not be the expected result when compositing two images with fully transparent backgrounds?
************************************************************************/
inline pixel4f_t blendOver(const pixel4f_t& fg, const pixel4f_t& bg)
{
    float aOut = clamp(fg.a + bg.b * (1.0f - fg.a), 0.001f, 1.0f);
    pixel4f_t pOut = (fg * fg.a + bg * bg.a * (1.0f - fg.a)) / aOut;
    pOut.a = aOut;
    return pOut;
}

/************************************************************************
* Pixel negative, not modifying alpha.
* Explicit name chosen for clarity instead of using operator-().
************************************************************************/
inline pixel4f_t negative(const pixel4f_t& pixel)
{
    return {
        1.0f - pixel.r,
        1.0f - pixel.g,
        1.0f - pixel.b,
        pixel.a
    };
}

/************************************************************************
* Fast brightness estimation.
* A more accurate version would convert to hsv and return v,
* but that is kinda slow.
************************************************************************/
inline float brightness(const pixel4f_t& pixel)
{
    return (pixel.r + pixel.g + pixel.b) / 3.0f;
}

/************************************************************************
* RGBA add, not modifying alpha. Prefer fg alpha.
************************************************************************/
inline pixel4f_t operator+(const pixel4f_t& fg, const pixel4f_t& bg)
{
    return {
        fg.r + bg.r,
        fg.g + bg.g,
        fg.b + bg.b,
        fg.a
    };
}

inline pixel4f_t operator-(const pixel4f_t& fg, const pixel4f_t& bg)
{
    return {
        fg.r - bg.r,
        fg.g - bg.g,
        fg.b - bg.b,
        fg.a
    };
}

/************************************************************************
* RGBA scale, only scales RGB channels.
************************************************************************/
inline pixel4f_t operator*(const float& scalar, const pixel4f_t& pixel)
{
    return {
        scalar * pixel.r,
        scalar * pixel.g,
        scalar * pixel.b,
        pixel.a
    };
}

inline pixel4f_t operator*(const pixel4f_t& pixel, const float& scalar)
{
    return {
        scalar * pixel.r,
        scalar * pixel.g,
        scalar * pixel.b,
        pixel.a
    };
}

inline pixel4f_t operator/(const pixel4f_t& pixel, const float& scalar)
{
    return {
        pixel.r / scalar,
        pixel.g / scalar,
        pixel.b / scalar,
        pixel.a
    };
}
    
inline pixel4f_t operator*(const pixel4f_t& pixel1, const pixel4f_t& pixel2)
{
    return {
        pixel1.r * pixel2.r,
        pixel1.g * pixel2.g,
        pixel1.b * pixel2.b,
        pixel1.a * pixel2.a,
    };
}
/************************************************************************
* Author: Kyle Bueche
* PixelI to pixelF conversion
*
* Evenly maps a discrete range [0, 255] to the continuous range [0, 1]
*
************************************************************************/
inline pixel4f_t pixelItoF(const pixel4i_t& pixel)
{
    return {
        (1.0f / 255.0f) * float(pixel.r),
        (1.0f / 255.0f) * float(pixel.g),
        (1.0f / 255.0f) * float(pixel.b),
        (1.0f / 255.0f) * float(pixel.a)
    };
}

/************************************************************************
* Author: Kyle Bueche
* Pixelf to pixelI conversion
*
* Evenly buckets a continuous range [0, 1] into the discrete range [0, 255].
* scaling by 255.99 rather than 255.0 provides a fair chance of truncating to 255.
*
************************************************************************/
inline pixel4i_t pixelFtoI(const pixel4f_t& pixel)
{
    return {
        uint8_t(255.99f * clamp(pixel.r, 0.0f, 1.0f)),
        uint8_t(255.99f * clamp(pixel.g, 0.0f, 1.0f)),
        uint8_t(255.99f * clamp(pixel.b, 0.0f, 1.0f)),
        uint8_t(255.99f * clamp(pixel.a, 0.0f, 1.0f))
    };
}

/************************************************************************
* Author: Kyle Bueche
* RGBA to HSVA conversion
*
* Algorithm adapted from rapidtables.com/convertor/rgb-to-hsv.html
*
************************************************************************/
inline pixel4f_hsv_t pixelRGBAtoHSVA(const pixel4f_t& pixel)
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
inline pixel4f_t pixelHSVAtoRGBA(const pixel4f_hsv_t& pixel)
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
