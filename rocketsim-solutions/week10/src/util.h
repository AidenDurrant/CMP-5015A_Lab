// util.h -- Week 10 (Easy): two function templates that replace code that was
// written once for double and once for Vec2.

#pragma once

#include <type_traits>

// Keep a number inside [low, high]. The static_assert (Hard 1) turns a page of
// template errors into one line when someone calls clamp on a Vec2.
template <typename T>
T clamp(T value, T low, T high)
{
    static_assert(std::is_arithmetic_v<T>, "clamp needs a number");
    return value < low ? low : (value > high ? high : value);
}

// Linear interpolation: a at t = 0, b at t = 1. Works for any T with + and *
// by a scalar, so for double and for Vec2 alike.
template <typename T, typename U>
T lerp(const T& a, const T& b, U t)
{
    return a + (b - a) * t;
}
