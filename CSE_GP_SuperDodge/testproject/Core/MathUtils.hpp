#pragma once

#include "MathTypes.hpp"
#include <cmath>

inline float ClampFloat(float value, float minValue, float maxValue)
{
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

inline bool IsOverlap(const Rect& a, const Rect& b)
{
    float ax = a.size.x * 0.5f;
    float ay = a.size.y * 0.5f;
    float bx = b.size.x * 0.5f;
    float by = b.size.y * 0.5f;

    return std::abs(a.center.x - b.center.x) <= ax + bx && std::abs(a.center.y - b.center.y) <= ay + by;
}

inline Vector2 Normalize(const Vector2& value)
{
    float length = std::sqrt(value.x * value.x + value.y * value.y);

    if (length <= 0.0001f)
        return Vector2(0.0f, 1.0f);

    return Vector2(value.x / length, value.y / length);
}