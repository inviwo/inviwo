#pragma once

#include "vec2_indexed.h"

#include <iostream>

class Edge
{
public:
    Edge(const Vec2Indexed &v1_, const Vec2Indexed &v2_) :
        v1(v1_), v2(v2_), isBad{false}
    {}

    Vec2Indexed v1;
    Vec2Indexed v2;

    bool isBad;
};

std::ostream &operator << (std::ostream &str, const Edge& e);
bool operator == (const Edge& e1, const Edge& e2);
bool almost_equal(const Edge& e1, const Edge& e2);
