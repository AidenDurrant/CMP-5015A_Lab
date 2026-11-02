// vec2.h -- a 2-D vector.
//
// Week 7: a plain struct plus free functions, written the way the C half of the
//         module wrote the Fraction ADT (lecture 7) but using C++ references.
//         These functions are GIVEN. Read them: every one takes its arguments by
//         const reference, which is how C++ passes "large" read-only values.
// Week 8: you will replace these free functions with operators (+, -, *, <<).

#pragma once

#include <cmath>

struct Vec2
{
    double x = 0.0;
    double y = 0.0;
};

// Sum of two vectors.
inline Vec2 add(const Vec2& a, const Vec2& b)
{
    Vec2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

// Difference a - b.
inline Vec2 subtract(const Vec2& a, const Vec2& b)
{
    Vec2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

// Vector multiplied by a scalar.
inline Vec2 scale(const Vec2& v, double k)
{
    Vec2 result;
    result.x = v.x * k;
    result.y = v.y * k;
    return result;
}

// Length (magnitude) of a vector.
inline double length(const Vec2& v)
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}

// A unit vector pointing the same way as v (or the zero vector if v is zero).
inline Vec2 normalised(const Vec2& v)
{
    const double len = length(v);
    if (len == 0.0)
    {
        return Vec2{};
    }
    return scale(v, 1.0 / len);
}

// A unit vector at the given angle (degrees, measured from the +x axis).
inline Vec2 unitFromAngle(double degrees)
{
    const double radians = degrees * 3.14159265358979323846 / 180.0;
    Vec2 result;
    result.x = std::cos(radians);
    result.y = std::sin(radians);
    return result;
}
