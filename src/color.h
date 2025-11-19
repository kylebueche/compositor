/************************************************************************
 * Author: Kyle Bueche
 * File: image.h
 *
 * Header with some inline functions for RGBA and HSVA operations.
 * Pixel functions are used in a greater ImagePipeline class.
************************************************************************/

#ifndef COLOR_H
#define COLOR_H

#include <cmath>
#include <numbers>
#include "math.h"

struct col4i_t
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct col4f_t
{
    float r;
    float g;
    float b;
    float a;
};

struct col4f_hsv_t
{
    float h;
    float s;
    float v;
    float a;
};

struct rgba_quad_t
{
    col4f_t topLeft;
    col4f_t topRight;
    col4f_t bottomLeft;
    col4f_t bottomRight;
};

// Standard col operations
inline col4f_t operator+(const col4f_t& fg, const col4f_t& bg);
inline col4f_t operator-(const col4f_t& fg, const col4f_t& bg);
inline col4f_t operator*(const float& scalar, const col4f_t& col);
inline col4f_t operator*(const col4f_t& col, const float& scalar);
inline col4f_t operator/(const col4f_t& col, const float& scalar);
inline col4f_t operator*(const col4f_t& col1, const col4f_t& col2);
inline col4f_t negate(const col4f_t& col1, const col4f_t& col2);

inline int clamp(int value, int min, int max);
inline int index(int x, int y, int width, int height);
inline col4f_hsv_t colRGBAtoHSVA(const col4f_t& col);
inline col4f_t colHSVAtoRGBA(const col4f_hsv_t& col);
inline col4f_t colItoF(const col4i_t& col);
inline col4i_t colFtoI(const col4f_t& col);
inline col4f_t lerp(float t, col4f_t t0, col4f_t t1)
inline col4f_t bicubic_interpolation(float tx, float ty, rgba_quad_t rgbaQuad);
inline col4f_t bilinear_interpolation(float tx, float ty, rgba_quad_t rgbaQuad);

/************************************************************************
* RGBA Blend adapted from Wikipedia
* TODO: Clamp is in place as a branchless safeguard against division by 0.
* May not be the expected result when compositing two images with fully transparent backgrounds?
************************************************************************/
inline col4f_t blendOver(const col4f_t& fg, const col4f_t& bg)
{
    float aOut = clamp(fg.a + bg.b * (1.0f - fg.a), 0.001f, 1.0f);
    col4f_t pOut = (fg * fg.a + bg * bg.a * (1.0f - fg.a)) / aOut;
    pOut.a = aOut;
    return pOut;
}

/************************************************************************
* Pixel negative, not modifying alpha.
* Explicit name chosen for clarity instead of using operator-().
************************************************************************/
inline col4f_t negative(const col4f_t& col)
{
    return {
        1.0f - col.r,
        1.0f - col.g,
        1.0f - col.b,
        col.a
    };
}

/************************************************************************
* Fast brightness estimation.
* A more accurate version would convert to hsv and return v,
* but that is kinda slow.
************************************************************************/
inline float brightness(const col4f_t& col)
{
    return (col.r + col.g + col.b) / 3.0f;
}

/************************************************************************
* RGBA add, not modifying alpha. Prefer fg alpha.
************************************************************************/
inline col4f_t operator+(const col4f_t& fg, const col4f_t& bg)
{
    return {
        fg.r + bg.r,
        fg.g + bg.g,
        fg.b + bg.b,
        fg.a
    };
}

inline col4f_t operator-(const col4f_t& fg, const col4f_t& bg)
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
inline col4f_t operator*(const float& scalar, const col4f_t& col)
{
    return {
        scalar * col.r,
        scalar * col.g,
        scalar * col.b,
        col.a
    };
}

inline col4f_t operator*(const col4f_t& col, const float& scalar)
{
    return {
        scalar * col.r,
        scalar * col.g,
        scalar * col.b,
        col.a
    };
}

inline col4f_t operator/(const col4f_t& col, const float& scalar)
{
    return {
        col.r / scalar,
        col.g / scalar,
        col.b / scalar,
        col.a
    };
}
    
inline col4f_t operator*(const col4f_t& col1, const col4f_t& col2)
{
    return {
        col1.r * col2.r,
        col1.g * col2.g,
        col1.b * col2.b,
        col1.a * col2.a,
    };
}
/************************************************************************
* Author: Kyle Bueche
* PixelI to colF conversion
*
* Evenly maps a discrete range [0, 255] to the continuous range [0, 1]
*
************************************************************************/
inline col4f_t colItoF(const col4i_t& col)
{
    return {
        (1.0f / 255.0f) * float(col.r),
        (1.0f / 255.0f) * float(col.g),
        (1.0f / 255.0f) * float(col.b),
        (1.0f / 255.0f) * float(col.a)
    };
}

/************************************************************************
* Author: Kyle Bueche
* Pixelf to colI conversion
*
* Evenly buckets a continuous range [0, 1] into the discrete range [0, 255].
* scaling by 255.99 rather than 255.0 provides a fair chance of truncating to 255.
*
************************************************************************/
inline col4i_t colFtoI(const col4f_t& col)
{
    return {
        uint8_t(255.99f * clamp(col.r, 0.0f, 1.0f)),
        uint8_t(255.99f * clamp(col.g, 0.0f, 1.0f)),
        uint8_t(255.99f * clamp(col.b, 0.0f, 1.0f)),
        uint8_t(255.99f * clamp(col.a, 0.0f, 1.0f))
    };
}

/************************************************************************
* Author: Kyle Bueche
* RGBA to HSVA conversion
*
* Algorithm adapted from rapidtables.com/convertor/rgb-to-hsv.html
*
************************************************************************/
inline col4f_hsv_t colRGBAtoHSVA(const col4f_t& col)
{
    col4f_hsv_t colOut;

    // Precomputation
    float c_max = std::max(std::max(col.r, col.g), col.b);
    float c_min = std::min(std::min(col.r, col.g), col.b);
    float delta = c_max - c_min;

    if (delta > 0.0f)
    {
        // Hue calculation
        if (c_max == col.r)
        {
            colOut.h = 60.0f * std::fmod(((col.g - col.b) / delta), 6.0f);
        }
        else if (c_max == col.g)
        {
            colOut.h = 60.0f * (((col.b - col.r) / delta) + 2.0f);
        }
        else if (c_max == col.b)
        {
            colOut.h = 60.0f * (((col.r - col.g) / delta) + 4.0f);
        }

        // Saturation calculation
        if (c_max > 0.0f)
        {
            colOut.s = delta / c_max;
        }
        else
        {
            colOut.s = 0.0f;
        }
    }

    else
    {
        colOut.h = 0.0f;
        colOut.s = 0.0f;
    }

    // Value calculation
    colOut.v = c_max;
    // Alpha
    colOut.a = col.a;

    if (colOut.h < 0)
    {
        colOut.h = 360.0f + colOut.h;
    }
    
    return colOut;
}

/************************************************************************
* Author: Kyle Bueche
* HSVA to RGBA conversion
*
* Algorithm adapted from rapidtables.com/convertor/hsv-to-rgb.html
*
************************************************************************/
inline col4f_t colHSVAtoRGBA(const col4f_hsv_t& col)
{
    col4f_t colOut;
    // Ensure 0 <= H < 360,
    //        0 <= S <= 1,
    //        0 <= V <= 1
    float H = std::fmod(col.h, 360.0f);
    if (H < 0.0f)
    {
        H = H + 360.0f;
    }
    float S = std::min(std::max(col.s, 0.0f), 1.0f);
    float V = std::min(std::max(col.v, 0.0f), 1.0f);

    // Precomputation
    float C = V * S;
    float X = C * (1 - std::abs(std::fmod(H / 60.0f, 2) - 1));
    float m = V - C;

    // RGB Calculation
    if (0.0f <= H && H < 60.0f)
    {
        colOut.r = C;
        colOut.g = X;
        colOut.b = 0.0f;
    }
    else if (60.0f <= H && H < 120.0f)
    {
        colOut.r = X;
        colOut.g = C;
        colOut.b = 0.0f;
    }
    else if (120.0f <= H && H < 180.0f)
    {
        colOut.r = 0.0f;
        colOut.g = C;
        colOut.b = X;
    }
    else if (180.0f <= H && H < 240.0f)
    {
        colOut.r = 0.0f;
        colOut.g = X;
        colOut.b = C;
    }
    else if (240.0f <= H && H < 300.0f)
    {
        colOut.r = X;
        colOut.g = 0.0f;
        colOut.b = C;
    }
    else if (300.0f <= H && H < 360.0f)
    {
        colOut.r = C;
        colOut.g = 0.0f;
        colOut.b = X;
    }
    else
    {
        colOut.r = 0.0f;
        colOut.g = 0.0f;
        colOut.b = 0.0f;
    }

    colOut.r += m;
    colOut.g += m;
    colOut.b += m;

    // Alpha
    colOut.a = col.a;

    return colOut;
}

inline col4f_t lerp(float t, col4f_t t0, col4f_t t1)
{
    return { lerp(t, t0.r, t1.r),
             lerp(t, t0.g, t1.g),
             lerp(t, t0.b, t1.b),
             lerp(t, t0.a, t1.a) };
}

// tx: 0 is left, ty: 0 is up
inline col4f_t bilinear_interpolation(float tx, float ty, rgba_quad_t rgbaQuad)
{
    col4f_t topColor = mylerp(tx, rgbaQuad.topLeft, rgbaQuad.topRight);
    col4f_t bottomColor = mylerp(tx, rgbaQuad.bottomLeft, rgbaQuad.bottomRight);
    col4f_t color = mylerp(ty, topColor, bottomColor);
    return color;
}

// tx: 0 is left, ty: 0 is up
inline col4f_t bicubic_interpolation(float tx, float ty, rgba_quad_t rgbaQuad)
{
    col4f_t topColor = cubic_interpolation(tx, rgbaQuad.topLeft, rgbaQuad.topRight);
    col4f_t bottomColor = cubic_interpolation(tx, rgbaQuad.bottomLeft, rgbaQuad.bottomRight);
    col4f_t color = cubic_interpolation(ty, topColor, bottomColor);
    return color;
}

#endif
