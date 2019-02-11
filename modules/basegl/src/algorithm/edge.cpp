#include <modules/basegl/algorithm/edge.h>

std::ostream &operator << (std::ostream &str, const Edge& e)
{
    return str << "Edge " << e.v1 << ", " << e.v2;
}

bool operator == (const Edge& e1, const Edge& e2)
{
    return (e1.v1 == e2.v1 && e1.v2 == e2.v2) ||
           (e1.v1 == e2.v2 && e1.v2 == e2.v1);
}

bool almost_equal(const Edge& e1, const Edge& e2)
{
    return (almost_equal(e1.v1, e2.v1) && almost_equal(e1.v2, e2.v2)) ||
           (almost_equal(e1.v1, e2.v2) && almost_equal(e1.v2, e2.v1));
}
