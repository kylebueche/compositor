/************************************************************************
 * Author: Kyle Bueche
 * File: math.h
 *
 * Header with some inline functions for RGBA and HSVA operations.
 * Pixel functions are used in a greater ImagePipeline class.
************************************************************************/

#ifndef MATH_H
#define MATH_H

#include <cmath>
#include <numbers>

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

inline float lerp(float t, float t0, float t1)
{
    return t * (t1 - t0) + t0;
}

inline float lerp(float t, int t0, int t1)
{
    return lerp(t, float(t0), float(t1));
}

inline float clerp(float t, float t0, float t1)
{
    return clamp(lerp(t, t0, t1), std::min(t0, t1), std::max(t0, t1));
}

inline vec2 lerp(float t, vec2 t0, vec2 t1)
{
    return { lerp(t, t0.x, t1.x),
             lerp(t, t0.y, t1.y) };
}

/*********************************************************************************
 * Uses the value of 4 points, t_-1, t_0, t_1, t_2, to smoothly interpolate
 * between t_0 and t_1.
 *
 * Some function:
 * f(tx) = a0(tx)^0 + a1(tx)^1 + a2(tx)^2 + a3(tx)^3
 * exists, which we want to pass through each above point.
 *
 * Then:
 *
 * f(-1) = a0(-1)^0 + a1(-1)^1 + a2(-1)^2 + a3(-1)^3
 * f(0)  = a0(0)^0  + a1(0)^1  + a2(0)^2  + a3(0)^3
 * f(1)  = a0(1)^0  + a1(1)^1  + a2(1)^2  + a3(1)^3
 * f(2)  = a0(2)^0  + a1(2)^1  + a2(2)^2  + a3(2)^3
 *
 * So, We construct the System of Equations:
 *
 * | f(-1)|   | 1 -1  1 -1 || a0 |
 * | f(0) |   | 1  0  0  0 || a1 |
 * | f(1) | = | 1  1  1  1 || a2 |
 * | f(2) |   | 1  2  4  8 || a3 |
 *
 * of the form y = Xa with solution a = (X^-1)y
 *
 * The solution is:
 *
 * | a0 |     | 0  6  0  0 || f(-1)|
 * | a1 |   1 |-2 -3  6 -1 || f(0) |
 * | a2 | = -*| 0 -6  3  0 || f(1) |
 * | a3 |   6 |-1  3 -3  1 || f(2) |
 *
 * which can be deconstructed into:
 *
 * a0 = (1/6) * (6f(0)) =                          f(0)
 * a1 = (1/6) * (-2f(-1) - 3f(0) + 6f(1) - f(2)) = -f(-1)/3 - f(0)/2 + f(1) -f(2)/6
 * a2 = (1/6) * (-6f(0) + 3f(1)) =                 -f(0) + f(1)/2
 * a3 = (1/6) * (-f(-1) + 3f(0) - 3f(1) + f(2)) =  -f(-1)/6 + f(0)/2 - f(1)/2 + f(2)/6
 *
 * And:
 * f(tx) = f(0)
 *       + t * (-f(-1)/3 - f(0)/2 + f(1) - f(2)/6)
 *       + t^2 * (-f(0) + f(1)/2)
 *       + t^2 * (-f(-1)/6 + f(0)/2 - f(1)/2 + f(2)/6)
 *
 ****************************************************************************************/
inline float cubic_interpolation(float t, float tneg1, float t0, float t1, float t2)
{
    float t0div2 = t0 / 2.0f;
    float t1div2 = t1 / 2.0f;
    float t2div6 = t2 / 6.0f;
    float answer = t0;
    answer += t * (tneg1 / 3.0f - t0div2 + t1 - t2div6);
    answer += t * t * (-t0 + t1div2);
    answer += t * t * t * (-tneg1 / 6.0f + t0div2 - t1div2 + t2div6);
    return answer;
}

inline col4f_t cubic_interpolation(float t, col4f_t tneg1, col4f_t t0, col4f_t t1, col4f_t t2)
{
    // -2x^3 + 3x^2 is a cubic function from 0, 0 to 1, 1, with slope 0 at each point.
    return { cubic_interpolation(t, tneg1.r, t0.r, t1.r, t2.r),
             cubic_interpolation(t, tneg1.g, t0.g, t1.g, t2.g),
             cubic_interpolation(t, tneg1.b, t0.b, t1.b, t2.b),
             cubic_interpolation(t, tneg1.a, t0.a, t1.a, t3.a) };
}


#endif
