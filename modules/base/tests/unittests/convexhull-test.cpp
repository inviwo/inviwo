/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

namespace inviwo {

// create a point set, but only for glm vector types
template <class T, typename std::enable_if<
                       util::rank<T>::value ==
                           1 /* && util::is_floating_point<T::value_type>::value == true*/,
                       int>::type = 0>
std::vector<T> getPointSet(const std::size_t numPoints) {
    srand(0);  // seed to always be the same random numbers

    std::vector<T> points(numPoints);
    for (size_t i = 0; i < numPoints; i++) {
        for (size_t j = 0; j < util::extent<T>::value; ++j) {
            points[i][j] = rand() / float(RAND_MAX);
        }
    }
    return points;
}

template <class T, typename std::enable_if<
                       util::rank<T>::value ==
                           1 /*&& util::is_floating_point<T::value_type>::value == false*/,
                       int>::type = 0>
std::vector<T> getPointSet(const std::size_t numPoints, T extent) {
    srand(0);  // seed to always be the same random numbers

    std::vector<T> points(numPoints);
    for (size_t i = 0; i < numPoints; i++) {
        for (size_t j = 0; j < util::extent<T>::value; ++j) {
            points[i][j] = rand() % extent[j];
        }
    }
    return points;
}

// TEST(ConvexHullTests, init) {
//}

TEST(isConvex, oneElement) {
    std::vector<ivec2> points = {ivec2(1, 0)};
    std::vector<ivec2> result = {ivec2(1, 0)};

    auto hull = util::convexHull(points);
    EXPECT_TRUE(util::isConvex(hull));
    EXPECT_EQ(result, hull) << "computed hull is _not_ convex";
}

TEST(isConvex, twoElements) {
    std::vector<ivec2> points = {ivec2(1, 0), ivec2(0, 0)};
    std::vector<ivec2> result = {ivec2(0, 0), ivec2(1, 0)};

    auto hull = util::convexHull(points);
    EXPECT_EQ(result, hull);
    EXPECT_TRUE(util::isConvex(hull)) << "computed hull is _not_ convex";
}

TEST(isConvex, threeElements) {
    std::vector<ivec2> points = {ivec2(1, 0), ivec2(0, 0), ivec2(1, 1)};
    std::vector<ivec2> result = {ivec2(0, 0), ivec2(1, 0), ivec2(1, 1)};

    auto hull = util::convexHull(points);
    EXPECT_EQ(result, hull);
    EXPECT_TRUE(util::isConvex(hull)) << "computed hull is _not_ convex";
}

TEST(isConvex, fourElements) {
    std::vector<ivec2> points = {ivec2(1, 0), ivec2(0, 0), ivec2(0, 1), ivec2(1, 1)};
    std::vector<ivec2> result = {ivec2(0, 0), ivec2(1, 0), ivec2(1, 1), ivec2(0, 1)};

    auto hull = util::convexHull(points);
    EXPECT_EQ(result, hull);
    EXPECT_TRUE(util::isConvex(hull)) << "computed hull is _not_ convex";
}

TEST(isConvex, fourElementsDouble) {
    std::vector<dvec2> points = {dvec2(1, 0), dvec2(0, 0), dvec2(0, 1), dvec2(1, 1)};
    std::vector<dvec2> result = {dvec2(0, 0), dvec2(1, 0), dvec2(1, 1), dvec2(0, 1)};

    auto hull = util::convexHull(points);
    EXPECT_EQ(result, hull);
    EXPECT_TRUE(util::isConvex(hull)) << "computed hull is _not_ convex";
}

TEST(isInside, inside) {
    std::vector<ivec2> points = {ivec2(2, 0), ivec2(0, 0), ivec2(0, 2), ivec2(2, 2)};
    std::vector<ivec2> result = {ivec2(0, 0), ivec2(2, 0), ivec2(2, 2), ivec2(0, 2)};

    auto hull = util::convexHull(points);
    EXPECT_EQ(result, hull);
    EXPECT_TRUE(util::isInside(hull, ivec2(1, 1))) << "point (1,1) not inside convex hull";
}

TEST(isInside, insideDouble) {
    std::vector<dvec2> points = {dvec2(1, 0), dvec2(0, 0), dvec2(0, 1), dvec2(1, 1)};
    std::vector<dvec2> result = {dvec2(0, 0), dvec2(1, 0), dvec2(1, 1), dvec2(0, 1)};

    auto hull = util::convexHull(points);
    EXPECT_EQ(result, hull);
    EXPECT_TRUE(util::isInside(hull, dvec2(0.5, 0.5))) << "point (0.5,0.5) not inside convex hull";
}

TEST(isInside, outsideDouble) {
    std::vector<dvec2> points = {dvec2(1, 0), dvec2(0, 0), dvec2(0, 1), dvec2(1, 1)};
    std::vector<dvec2> result = {dvec2(0, 0), dvec2(1, 0), dvec2(1, 1), dvec2(0, 1)};

    auto hull = util::convexHull(points);
    EXPECT_EQ(result, hull);
    EXPECT_FALSE(util::isInside(hull, dvec2(1.5, 0.5)))
        << "point (1.5,0.5) not outside convex hull";
    EXPECT_FALSE(util::isInside(hull, dvec2(-0.25, 0.5)))
        << "point (-0.25,0.5) not outside convex hull";
}

TEST(convexHull, ivec2) {
    auto points = getPointSet<ivec2>(10, ivec2(10, 10));
    auto hull = util::convexHull(points);

    EXPECT_TRUE(util::isConvex(hull)) << "computed hull is _not_ convex (monotone chain)";
}

TEST(convexHull, dvec2) {
    auto points = getPointSet<dvec2>(10);
    auto hull = util::convexHull(points);

    EXPECT_TRUE(util::isConvex(hull));
}

TEST(convexHull, vec3) {
    std::vector<vec3> p = {vec3(0.0f), vec3(1.0f)};
    // convex hull is not yet implemented for other types than *vec2
    EXPECT_THROW(util::convexHull<vec3>(p), inviwo::Exception);
}

}  // namespace inviwo
