/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <modules/base/algorithm/convexhull.h>

#include <array>
#include <numbers>
#include <ranges>

namespace inviwo {

namespace {

// create a point set, but only for glm vector types
template <util::Vec2D T, size_t N>
std::array<T, N> getPointSet() {
    srand(0);  // seed to always be the same random numbers

    std::array<T, N> points{};
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < util::extent<T>::value; ++j) {
            points[i][j] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        }
    }
    return points;
}

template <util::Vec2D T, size_t N>
std::array<T, N> getPointSet(T extent) {
    srand(0);  // seed to always be the same random numbers

    std::array<T, N> points{};
    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < util::extent<T>::value; ++j) {
            points[i][j] = static_cast<typename T::value_type>(rand()) % extent[j];
        }
    }
    return points;
}

}  // namespace

TEST(isConvex, oneElement) {
    const std::vector<ivec2> points = {ivec2(1, 0)};
    const std::vector<ivec2> result = {ivec2(1, 0)};

    const auto hull = util::convexHull(points);
    EXPECT_TRUE(util::isConvex(hull));
    EXPECT_EQ(result, hull) << "computed hull is _not_ convex";
}

TEST(isConvex, twoElements) {
    const std::vector<ivec2> points = {ivec2(1, 0), ivec2(0, 0)};
    const std::vector<ivec2> result = {ivec2(0, 0), ivec2(1, 0)};

    const auto hull = util::convexHull(points);
    EXPECT_EQ(result, hull);
    EXPECT_TRUE(util::isConvex(hull)) << "computed hull is _not_ convex";
}

TEST(isConvex, threeElements) {
    const std::vector<ivec2> points = {ivec2(1, 0), ivec2(0, 0), ivec2(1, 1)};
    const std::vector<ivec2> result = {ivec2(0, 0), ivec2(1, 0), ivec2(1, 1)};

    const auto hull = util::convexHull(points);
    EXPECT_EQ(result, hull);
    EXPECT_TRUE(util::isConvex(hull)) << "computed hull is _not_ convex";
}

TEST(isConvex, fourElements) {
    const std::vector<ivec2> points = {ivec2(1, 0), ivec2(0, 0), ivec2(0, 1), ivec2(1, 1)};
    const std::vector<ivec2> result = {ivec2(0, 0), ivec2(1, 0), ivec2(1, 1), ivec2(0, 1)};

    const auto hull = util::convexHull(points);
    EXPECT_EQ(result, hull);
    EXPECT_TRUE(util::isConvex(hull)) << "computed hull is _not_ convex";
}

TEST(isConvex, fourElementsDouble) {
    const std::vector<dvec2> points = {dvec2(1, 0), dvec2(0, 0), dvec2(0, 1), dvec2(1, 1)};
    const std::vector<dvec2> result = {dvec2(0, 0), dvec2(1, 0), dvec2(1, 1), dvec2(0, 1)};

    const auto hull = util::convexHull(points);
    EXPECT_EQ(result, hull);
    EXPECT_TRUE(util::isConvex(hull)) << "computed hull is _not_ convex";
}

TEST(isInside, inside) {
    const std::vector<ivec2> points = {ivec2(2, 0), ivec2(0, 0), ivec2(0, 2), ivec2(2, 2)};
    const std::vector<ivec2> result = {ivec2(0, 0), ivec2(2, 0), ivec2(2, 2), ivec2(0, 2)};

    const auto hull = util::convexHull(points);
    EXPECT_EQ(result, hull);
    EXPECT_TRUE(util::isInside(hull, ivec2(1, 1))) << "point (1,1) not inside convex hull";
}

TEST(isInside, insideDouble) {
    const std::vector<dvec2> points = {dvec2(1, 0), dvec2(0, 0), dvec2(0, 1), dvec2(1, 1)};
    const std::vector<dvec2> result = {dvec2(0, 0), dvec2(1, 0), dvec2(1, 1), dvec2(0, 1)};

    const auto hull = util::convexHull(points);
    EXPECT_EQ(result, hull);
    EXPECT_TRUE(util::isInside(hull, dvec2(0.5, 0.5))) << "point (0.5,0.5) not inside convex hull";
}

TEST(isInside, outsideDouble) {
    const std::vector<dvec2> points = {dvec2(1, 0), dvec2(0, 0), dvec2(0, 1), dvec2(1, 1)};
    const std::vector<dvec2> result = {dvec2(0, 0), dvec2(1, 0), dvec2(1, 1), dvec2(0, 1)};

    const auto hull = util::convexHull(points);
    EXPECT_EQ(result, hull);
    EXPECT_FALSE(util::isInside(hull, dvec2(1.5, 0.5)))
        << "point (1.5,0.5) not outside convex hull";
    EXPECT_FALSE(util::isInside(hull, dvec2(-0.25, 0.5)))
        << "point (-0.25,0.5) not outside convex hull";
}

TEST(convexHull, ivec2) {
    const auto points = getPointSet<ivec2, 10>({10, 10});
    const auto hull = util::convexHull(points);

    EXPECT_TRUE(util::isConvex(hull)) << "computed hull is _not_ convex (monotone chain)";
    for (const auto& p : points) {
        EXPECT_TRUE(util::isInside(hull, p)) << "point p=" << p << " outside of convex hull ";
    }
}

TEST(convexHull, dvec2) {
    const auto points = getPointSet<dvec2, 10>();
    const auto hull = util::convexHull(points);

    EXPECT_TRUE(util::isConvex(hull));
}

TEST(convexHull, dvec2span) {
    const auto points = getPointSet<dvec2, 10>();

    std::array<dvec2, points.size() + 1> hull{};
    auto result = util::convexHull<10, double>(points, hull);

    EXPECT_TRUE(util::isConvex(result));
    for (const auto& p : points) {
        EXPECT_TRUE(util::isInside(result, p)) << "point p=" << p << " outside of convex hull";
    }
}

TEST(convexHull, ivec2span) {
    const auto points = getPointSet<ivec2, 10>({10, 10});

    std::array<ivec2, points.size() + 1> hull{};
    auto result = util::convexHull<10, std::int32_t>(points, hull);

    EXPECT_TRUE(util::isConvex(result));
    for (const auto& p : points) {
        EXPECT_TRUE(util::isInside(result, p)) << "point p=" << p << " outside of convex hull";
    }
}

TEST(convexHull, vec3) {
    const std::vector<vec3> p = {vec3{0.0f}, vec3{1.0f}};
    // convex hull is not yet implemented for other types than *vec2
    EXPECT_THROW(util::convexHull(p), inviwo::Exception);
}

namespace {

template <typename T, size_t N>
std::array<T, N> rotate(double angleRadian, const std::array<T, N>& points) {
    std::array<T, N> result{};
    const dmat2 m{std::cos(angleRadian), -std::sin(angleRadian), std::sin(angleRadian),
                  std::cos(angleRadian)};
    std::ranges::transform(points, result.begin(), [m](auto& pos) { return m * pos; });
    return result;
}

template <size_t N>
std::array<dvec2, N> createNgon(const dvec2& center, double radius) {
    std::array<dvec2, N> points = {dvec2{0.0}};

    const double deltaAngle = std::numbers::pi * 2.0 / static_cast<double>(N);
    for (auto i : std::views::iota(size_t{0}, points.size())) {
        const auto angle = deltaAngle * static_cast<double>(i);
        points[i] = center + dvec2{std::cos(angle), std::sin(angle)} * radius;
    }
    return points;
}

double ngonArea(size_t n, double r) {
    return static_cast<double>(n) / 2.0 *
           std::sin(2.0 * std::numbers::pi / static_cast<double>(n)) * r * r;
}

}  // namespace

TEST(polygonArea, unitSquare) {
    const auto p = std::to_array<dvec2>({{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}});

    const double expected = 1.0;
    EXPECT_DOUBLE_EQ(expected, util::getArea(p));

    const std::array p2 = rotate(std::numbers::pi / 3.0, p);
    EXPECT_DOUBLE_EQ(expected, util::getArea(p2));
}

TEST(polygonArea, squareCCW) {
    const auto p = std::to_array<dvec2>({{-0.5, -0.5}, {0.5, -0.5}, {0.5, 0.5}, {-0.5, 0.5}});

    const double expected = 1.0;
    EXPECT_DOUBLE_EQ(expected, util::getArea(p));

    const std::array p2 = rotate(std::numbers::pi / 3.0, p);
    EXPECT_DOUBLE_EQ(expected, util::getArea(p2));
}

TEST(polygonArea, squareCW) {
    const auto p = std::to_array<dvec2>({{-0.5, -0.5}, {-0.5, 0.5}, {0.5, 0.5}, {0.5, -0.5}});

    const double expected = 1.0;
    EXPECT_DOUBLE_EQ(expected, util::getArea(p));

    const std::array p2 = rotate(std::numbers::pi / 3.0, p);
    EXPECT_DOUBLE_EQ(expected, util::getArea(p2));
}

TEST(polygonArea, triangle) {
    const auto p = std::to_array<dvec2>({{-0.5, 0.0}, {0.5, 0.0}, {0.0, 0.5}});

    const double expected = 1.0 * 0.5 * 0.5;
    EXPECT_DOUBLE_EQ(expected, util::getArea(p));

    const std::array p2 = rotate(std::numbers::pi / 3.0, p);
    EXPECT_DOUBLE_EQ(expected, util::getArea(p2));
}

TEST(polygonArea, rightTriangle) {
    const auto p = std::to_array<dvec2>({{1.4, 0.0}, {1.9, 0.0}, {1.4, 1.0}});

    const double expected = 1.0 * 0.5 * 0.5;
    EXPECT_DOUBLE_EQ(expected, util::getArea(p));

    const std::array p2 = rotate(std::numbers::pi / 3.0, p);
    EXPECT_DOUBLE_EQ(expected, util::getArea(p2));
}

TEST(polygonArea, hexagon) {
    const double r = 1.5;
    const auto points = createNgon<6>(dvec2{1.5, 2.3}, r);

    const double expected = ngonArea(6, r);
    EXPECT_DOUBLE_EQ(expected, util::getArea(points));

    const std::array points2 = rotate(std::numbers::pi / 3.0, points);
    EXPECT_DOUBLE_EQ(expected, util::getArea(points2));
}

TEST(polygonArea, octagon) {
    const double r = 1.5;
    const auto points = createNgon<8>(dvec2{2.0, 1.5}, r);

    const double expected = ngonArea(8, r);
    EXPECT_DOUBLE_EQ(expected, util::getArea(points));

    const std::array points2 = rotate(std::numbers::pi / 3.0, points);
    EXPECT_DOUBLE_EQ(expected, util::getArea(points2));
}

}  // namespace inviwo
