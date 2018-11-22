#pragma once

#include "vec2_indexed.h"

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

inline std::ostream &operator << (std::ostream &str, const Edge& e)
{
    return str << "Edge " << e.v1 << ", " << e.v2;
}

inline bool operator == (const Edge& e1, const Edge& e2)
{
    return (e1.v1 == e2.v1 && e1.v2 == e2.v2) ||
           (e1.v1 == e2.v2 && e1.v2 == e2.v1);
}

inline bool almost_equal(const Edge& e1, const Edge& e2)
{
    return (almost_equal(e1.v1, e2.v1) && almost_equal(e1.v2, e2.v2)) ||
           (almost_equal(e1.v1, e2.v2) && almost_equal(e1.v2, e2.v1));
}
