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

struct col4i
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct col4f
{
    col4f() {}
    col4f(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
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
    col4f topLeft;
    col4f topRight;
    col4f bottomLeft;
    col4f bottomRight;
};

struct rgba_grid_t
{
    col4f _11;
    col4f _12;
    col4f _13;
    col4f _14;

    col4f _21;
    col4f _22;
    col4f _23;
    col4f _24;

    col4f _31;
    col4f _32;
    col4f _33;
    col4f _34;

    col4f _41;
    col4f _42;
    col4f _43;
    col4f _44;
};

// Standard col operations
inline col4f operator+(const col4f& fg, const col4f& bg);
inline col4f operator-(const col4f& fg, const col4f& bg);
inline col4f operator*(const float& scalar, const col4f& col);
inline col4f operator*(const col4f& col, const float& scalar);
inline col4f operator/(const col4f& col, const float& scalar);
inline col4f operator*(const col4f& col1, const col4f& col2);
inline col4f negate(const col4f& col1, const col4f& col2);

inline int index(int x, int y, int width, int height);
inline col4f_hsv_t colRGBAtoHSVA(const col4f& col);
inline col4f colHSVAtoRGBA(const col4f_hsv_t& col);
inline col4f colItoF(const col4i& col);
inline col4i colFtoI(const col4f& col);
inline col4f linear_interpolation(float t, col4f t0, col4f t1);
inline col4f cubic_interpolation(float t, col4f tneg1, col4f t0, col4f t1, col4f t2);
inline col4f bilinear_interpolation(float tx, float ty, rgba_quad_t rgbaQuad);
inline col4f bicubic_interpolation(float tx, float ty, rgba_grid_t rgbaGrid);
inline col4f nearest_neighbor(float tx, float ty, rgba_quad_t rgbaQuad);

/************************************************************************
* RGBA Blend adapted from Wikipedia
* TODO: Clamp is in place as a branchless safeguard against division by 0.
* May not be the expected result when compositing two images with fully transparent backgrounds?
************************************************************************/
inline col4f blendOver(const col4f& fg, const col4f& bg)
{
    float aOut = clamp(fg.a + bg.b * (1.0f - fg.a), 0.001f, 1.0f);
    col4f pOut = (fg * fg.a + bg * bg.a * (1.0f - fg.a)) / aOut;
    pOut.a = aOut;
    return pOut;
}

/************************************************************************
* Pixel negative, not modifying alpha.
* Explicit name chosen for clarity instead of using operator-().
************************************************************************/
inline col4f negative(const col4f& col)
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
inline float brightness(const col4f& col)
{
    return (col.r + col.g + col.b) / 3.0f;
}

/************************************************************************
* RGBA add, not modifying alpha. Prefer fg alpha.
************************************************************************/
inline col4f operator+(const col4f& fg, const col4f& bg)
{
    return {
        fg.r + bg.r,
        fg.g + bg.g,
        fg.b + bg.b,
        fg.a
    };
}

inline col4f operator-(const col4f& fg, const col4f& bg)
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
inline col4f operator*(const float& scalar, const col4f& col)
{
    return {
        scalar * col.r,
        scalar * col.g,
        scalar * col.b,
        col.a
    };
}

inline col4f operator*(const col4f& col, const float& scalar)
{
    return {
        scalar * col.r,
        scalar * col.g,
        scalar * col.b,
        col.a
    };
}

inline col4f operator/(const col4f& col, const float& scalar)
{
    return {
        col.r / scalar,
        col.g / scalar,
        col.b / scalar,
        col.a
    };
}
    
inline col4f operator*(const col4f& col1, const col4f& col2)
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
inline col4f colItoF(const col4i& col)
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
inline col4i colFtoI(const col4f& col)
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
inline col4f_hsv_t colRGBAtoHSVA(const col4f& col)
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
inline col4f colHSVAtoRGBA(const col4f_hsv_t& col)
{
    col4f colOut;
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

inline col4f linear_interpolation(float t, col4f t0, col4f t1)
{
    return { linear_interpolation(t, t0.r, t1.r),
             linear_interpolation(t, t0.g, t1.g),
             linear_interpolation(t, t0.b, t1.b),
             linear_interpolation(t, t0.a, t1.a) };
}

inline col4f cubic_interpolation(float t, col4f tneg1, col4f t0, col4f t1, col4f t2)
{
    return { cubic_interpolation(t, tneg1.r, t0.r, t1.r, t2.r),
             cubic_interpolation(t, tneg1.g, t0.g, t1.g, t2.g),
             cubic_interpolation(t, tneg1.b, t0.b, t1.b, t2.b),
             cubic_interpolation(t, tneg1.a, t0.a, t1.a, t2.a) };
}


// tx: 0 is left, ty: 0 is up
inline col4f bilinear_interpolation(float tx, float ty, rgba_quad_t rgbaQuad)
{
    col4f topColor = linear_interpolation(tx, rgbaQuad.topLeft, rgbaQuad.topRight);
    col4f bottomColor = linear_interpolation(tx, rgbaQuad.bottomLeft, rgbaQuad.bottomRight);
    col4f color = linear_interpolation(ty, topColor, bottomColor);
    return color;
}

// tx: 0 is left, ty: 0 is up
inline col4f bicubic_interpolation(float tx, float ty, rgba_grid_t rgbaGrid)
{
    col4f color_1 = cubic_interpolation(tx, rgbaGrid._11, rgbaGrid._12, rgbaGrid._13, rgbaGrid._14);
    col4f color_2 = cubic_interpolation(tx, rgbaGrid._21, rgbaGrid._22, rgbaGrid._23, rgbaGrid._24);
    col4f color_3 = cubic_interpolation(tx, rgbaGrid._31, rgbaGrid._32, rgbaGrid._33, rgbaGrid._34);
    col4f color_4 = cubic_interpolation(tx, rgbaGrid._41, rgbaGrid._42, rgbaGrid._43, rgbaGrid._44);
    col4f color = cubic_interpolation(ty, color_1, color_2, color_3, color_4);
    return color;
}

// tx: 0 is left, ty: 0 is up
// branchless nearest neighbor
inline col4f nearest_neighbor(float tx, float ty, rgba_quad_t rgbaQuad)
{
    col4f topColor = (tx < 0.5f) * rgbaQuad.topLeft + (tx >= 0.5f) * rgbaQuad.topRight;
    col4f bottomColor = (tx < 0.5f) * rgbaQuad.bottomLeft + (tx >= 0.5f) * rgbaQuad.bottomRight;
    col4f color = (ty < 0.5f) * topColor + (ty >= 0.5f) * bottomColor;
    return color;
}

#endif
