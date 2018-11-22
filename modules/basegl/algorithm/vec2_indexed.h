#pragma once

#include <glm/glm.hpp>
#include <limits>
#include "numeric.h"

class Vec2Indexed
{
public:
    Vec2Indexed(const glm::vec2& v_ = glm::vec2{0.0f}, size_t idx_ = std::numeric_limits<size_t>::max()) :
        v(v_), idx(idx_)
    {}

    Vec2Indexed(float x_, float y_, size_t idx_ = std::numeric_limits<size_t>::max()) :
        v(x_, y_), idx(idx_)
    {}

    Vec2Indexed(const Vec2Indexed& other) : v(other.v), idx(other.idx) {}
    Vec2Indexed(Vec2Indexed&& other) : v(other.v), idx(other.idx) {}
    Vec2Indexed& operator=(const Vec2Indexed& other) {
        v = other.v;
        idx = other.idx;
        return *this;
    }
    Vec2Indexed& operator=(Vec2Indexed&& other) {
        v = other.v;
        idx = other.idx;
        return *this;
    }

    union
    {
        struct
        {
            float x;
            float y;
        };
        glm::vec2 v;
    };
    size_t idx;

    float norm2() const
    {
        return glm::dot(v, v);
    }

    float dist2(const Vec2Indexed& other) const
    {
        const auto diff = v - other.v;
        return glm::dot(diff, diff);
    }

    float dist(const Vec2Indexed& other) const
    {
        return glm::sqrt(dist2(other));
    }
};

bool operator == (const Vec2Indexed& v1, const Vec2Indexed& v2)
{
    // perhaps use "idx" here
    return v1.v == v2.v;
    // return v1.idx == v2.idx;
}

bool almost_equal(const Vec2Indexed& v1, const Vec2Indexed& v2, int ulp = 2)
{
    return almost_equal(v1.x, v2.x, ulp) && almost_equal(v1.y, v2.y, ulp);
}

std::ostream &operator << (std::ostream &str, Vec2Indexed const& pt)
{
    return str << "Point (" << pt.idx << ") x: " << pt.x << " y: " << pt.y;
}
