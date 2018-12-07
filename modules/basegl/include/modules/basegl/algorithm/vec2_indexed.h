#pragma once

#include <glm/glm.hpp>
#include <iostream>

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
    Vec2Indexed& operator=(const Vec2Indexed& other);
    Vec2Indexed& operator=(Vec2Indexed&& other);

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

    float norm2() const;
    float dist2(const Vec2Indexed& other) const;
    float dist(const Vec2Indexed& other) const;
};

bool operator == (const Vec2Indexed& v1, const Vec2Indexed& v2);
bool almost_equal(const Vec2Indexed& v1, const Vec2Indexed& v2, int ulp = 2);
std::ostream &operator << (std::ostream &str, Vec2Indexed const& pt);
