#include "triangle.h"

bool Triangle::containsVertex(const Vec2Indexed& v) const {
    // return p1 == v || p2 == v || p3 == v;
    return almost_equal(p1, v) || almost_equal(p2, v) || almost_equal(p3, v);
}

bool Triangle::circumCircleContains(const Vec2Indexed& v) const {
    const auto ab = p1.norm2();
    const auto cd = p2.norm2();
    const auto ef = p3.norm2();

    const auto circum_x = (ab * (p3.y - p2.y) + cd * (p1.y - p3.y) + ef * (p2.y - p1.y)) /
                          (p1.x * (p3.y - p2.y) + p2.x * (p1.y - p3.y) + p3.x * (p2.y - p1.y));
    const auto circum_y = (ab * (p3.x - p2.x) + cd * (p1.x - p3.x) + ef * (p2.x - p1.x)) /
                          (p1.y * (p3.x - p2.x) + p2.y * (p1.x - p3.x) + p3.y * (p2.x - p1.x));

    const Vec2Indexed circum(0.5f * glm::vec2{circum_x, circum_y},
                                std::numeric_limits<size_t>::max());
    const auto circum_radius = p1.dist2(circum);
    const auto dist = v.dist2(circum);

    return dist <= circum_radius;
}

std::ostream& operator<<(std::ostream& str, const Triangle& t) {
    return str << "Triangle:" << std::endl
               << "\t" << t.p1 << std::endl
               << "\t" << t.p2 << std::endl
               << "\t" << t.p3 << std::endl
               << "\t" << t.e1 << std::endl
               << "\t" << t.e2 << std::endl
               << "\t" << t.e3 << std::endl;
}

bool operator==(const Triangle& t1, const Triangle& t2) {
    return (t1.p1 == t2.p1 || t1.p1 == t2.p2 || t1.p1 == t2.p3) &&
           (t1.p2 == t2.p1 || t1.p2 == t2.p2 || t1.p2 == t2.p3) &&
           (t1.p3 == t2.p1 || t1.p3 == t2.p2 || t1.p3 == t2.p3);
}

bool almost_equal(const Triangle& t1, const Triangle& t2) {
    return (almost_equal(t1.p1, t2.p1) || almost_equal(t1.p1, t2.p2) ||
            almost_equal(t1.p1, t2.p3)) &&
           (almost_equal(t1.p2, t2.p1) || almost_equal(t1.p2, t2.p2) ||
            almost_equal(t1.p2, t2.p3)) &&
           (almost_equal(t1.p3, t2.p1) || almost_equal(t1.p3, t2.p2) || almost_equal(t1.p3, t2.p3));
}
