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


struct vec2
{
    float x;
    float y;
};

inline vec2 operator+(const vec2& a, const vec2& b) { return { a.x + b.x, a.y + b.y }; }
inline vec2 operator-(const vec2& a, const vec2& b) { return { a.x - b.x, a.y - b.y }; }
inline vec2 operator*(const float& a, const vec2& b) { return { a * b.x, a * b.y }; }
inline vec2 operator*(const vec2& a, const float& b) { return { a.x * b, a.y * b }; }
inline float dot(const vec2& a, const vec2& b) { return a.x * b.x + a.y * b.y; };
inline float length2(const vec2& a) { return dot(a, a); };
inline float length(const vec2& a) { return sqrt(length2(a)); };
inline float distance(const vec2& a, const vec2& b) { return length(a - b); };
inline vec2 normalized(const vec2& a) { return (1.0f / length(a)) * a; };

inline vec2 vecScale(const vec2& scale, const vec2& a)
{
    return { scale.x * a.x, scale.y * a.y };
}

/* Potential optimization:
 * Possibly cache sin_theta and cos_theta for long streams of this operation
 */
inline vec2 vecRotate(const float& radians, const vec2& a)
{
    float sin_theta = sin(radians);
    float cos_theta = cos(radians);
    return {
        cos_theta * a.x - sin_theta * a.y,
        sin_theta * a.x + cos_theta * a.y
    };
}

inline vec2 vecTranslate(const vec2& translation, const vec2& a)
{
    return { translation.x + a.x, translation.y + a.y };
}


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

inline float linear_interpolation(float t, float t0, float t1)
{
    return t * (t1 - t0) + t0;
}

inline float linear_interpolation(float t, int t0, int t1)
{
    return linear_interpolation(t, float(t0), float(t1));
}

inline float clerp(float t, float t0, float t1)
{
    return clamp(linear_interpolation(t, t0, t1), std::min(t0, t1), std::max(t0, t1));
}

inline vec2 linear_interpolation(float t, vec2 t0, vec2 t1)
{
    return { linear_interpolation(t, t0.x, t1.x),
             linear_interpolation(t, t0.y, t1.y) };
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



#endif
