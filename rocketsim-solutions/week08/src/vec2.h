// vec2.h -- a 2-D vector. Week 8: the free functions add/subtract/scale from
// week 7 are now operators, so physics reads like the maths.

#pragma once

#include <cmath>
#include <ostream>

struct Vec2
{
    double x = 0.0;
    double y = 0.0;

    Vec2& operator+=(const Vec2& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2& operator-=(const Vec2& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vec2& operator*=(double k)
    {
        x *= k;
        y *= k;
        return *this;
    }
};

inline Vec2 operator+(Vec2 a, const Vec2& b) { return a += b; }
inline Vec2 operator-(Vec2 a, const Vec2& b) { return a -= b; }
inline Vec2 operator*(Vec2 v, double k) { return v *= k; }
inline Vec2 operator*(double k, Vec2 v) { return v *= k; }
inline Vec2 operator/(Vec2 v, double k) { return v *= (1.0 / k); }
inline Vec2 operator-(const Vec2& v) { return Vec2{-v.x, -v.y}; }

inline bool operator==(const Vec2& a, const Vec2& b) { return a.x == b.x && a.y == b.y; }
inline bool operator!=(const Vec2& a, const Vec2& b) { return !(a == b); }

inline std::ostream& operator<<(std::ostream& out, const Vec2& v)
{
    return out << '(' << v.x << ", " << v.y << ')';
}

inline double length(const Vec2& v) { return std::sqrt(v.x * v.x + v.y * v.y); }

inline Vec2 normalised(const Vec2& v)
{
    const double len = length(v);
    return len == 0.0 ? Vec2{} : v / len;
}

inline Vec2 unitFromAngle(double degrees)
{
    const double radians = degrees * 3.14159265358979323846 / 180.0;
    return Vec2{std::cos(radians), std::sin(radians)};
}
