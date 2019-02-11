#pragma once

#include <modules/basegl/algorithm/vec2_indexed.h>
#include <modules/basegl/algorithm/edge.h>
#include <modules/basegl/algorithm/numeric.h>

#include <iostream>

class Triangle
{
public:
    Triangle(const Vec2Indexed& p1_, const Vec2Indexed& p2_, const Vec2Indexed& p3_) :
        p1(p1_), p2(p2_), p3(p3_),
        e1(p1_, p2_), e2(p2_, p3_), e3(p3_, p1_),
        isBad{false}
    {}

    bool containsVertex(const Vec2Indexed& v) const;
    bool circumCircleContains(const Vec2Indexed& v) const;

    Vec2Indexed p1;
    Vec2Indexed p2;
    Vec2Indexed p3;
    Edge e1;
    Edge e2;
    Edge e3;
    bool isBad;
};

std::ostream &operator << (std::ostream &str, const Triangle& t);
bool operator == (const Triangle &t1, const Triangle &t2);
bool almost_equal(const Triangle &t1, const Triangle &t2);
