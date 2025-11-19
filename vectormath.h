/************************************************************************
 * Author: Kyle Bueche
 * File: vectormath.h
 *
 * Header with some inline functions for RGBA and HSVA operations.
 * Pixel functions are used in a greater ImagePipeline class.
************************************************************************/

struct vec2
{
    float x;
    float y;

    vec2(float x, float y) : x(x), y(y) {}
};

inline vec2 operator*(const float& scalar, const vec2& u)
{
    return vec2(scalar * u.x, scalar * u.y);
}

inline vec2 operator*(const vec2& u, const float& scalar)
{
    return vec2(scalar * u.x, scalar * u.y);
}

inline vec2 operator+(const vec2& u, const vec2& v)
{
    return vec2(u.x + v.x, u.y + v.y);
}

inline vec2 operator-(const vec2& u, const vec2& v)
{
    return vec2(u.x - v.x, u.y - v.y);
}

inline float dot(const vec2& u, const vec2& v)
{
    return u.x * v.x + u.y * v.y;
}

inline float length2(const vec2& u)
{
    return dot(u, u);
}

inline float length(const vec2& u)
{
    return sqrt(dot(u, u));
}

inline vec2 normalized(const vec2& u)
{
    return (1.0f / length(u)) * u;
}

