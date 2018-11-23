#pragma once

#include "vec2_indexed.h"
#include "numeric.h"

#include <limits>

Vec2Indexed& Vec2Indexed::operator=(const Vec2Indexed& other) {
    v = other.v;
    idx = other.idx;
    return *this;
}
Vec2Indexed& Vec2Indexed::operator=(Vec2Indexed&& other) {
    v = other.v;
    idx = other.idx;
    return *this;
}

float Vec2Indexed::norm2() const
{
    return glm::dot(v, v);
}

float Vec2Indexed::dist2(const Vec2Indexed& other) const
{
    const auto diff = v - other.v;
    return glm::dot(diff, diff);
}

float Vec2Indexed::dist(const Vec2Indexed& other) const
{
    return glm::sqrt(dist2(other));
}

bool operator == (const Vec2Indexed& v1, const Vec2Indexed& v2)
{
    // perhaps use "idx" here
    return v1.v == v2.v;
    // return v1.idx == v2.idx;
}

bool almost_equal(const Vec2Indexed& v1, const Vec2Indexed& v2, int ulp)
{
    return almost_equal(v1.x, v2.x, ulp) && almost_equal(v1.y, v2.y, ulp);
}

std::ostream &operator << (std::ostream &str, Vec2Indexed const& pt)
{
    return str << "Point (" << pt.idx << ") x: " << pt.x << " y: " << pt.y;
}
