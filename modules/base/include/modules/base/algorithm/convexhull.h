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

#pragma once

#include <modules/base/basemoduledefine.h>

#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/glm.h>

#include <vector>
#include <algorithm>
#include <span>

namespace inviwo::util {

template <typename T>
concept Vec2D = util::rank<T>::value == 1 && util::extent<T>::value == 2;

template <typename C, size_t N>
concept RangeOfVec =
    std::ranges::random_access_range<C> && util::rank<std::ranges::range_value_t<C>>::value == 1 &&
    util::extent<std::ranges::range_value_t<C>>::value == N;

/**
 * \brief check whether the given polygon is convex
 *
 * @param polygon   polygon consisting of points
 * @return true if the polygon is convex, false otherwise
 */
template <RangeOfVec<2> T>
bool isConvex(const T& polygon) {
    using E = std::ranges::range_value_t<T>;
    const std::size_t n = polygon.size();
    if (n < 3) return true;

    auto cross2D = [](const E& a, const E& b) { return (a.x * b.y - a.y * b.x); };

    for (std::size_t i = 0; i < n; ++i) {
        if (cross2D(polygon[(i + 1) % n] - polygon[i], polygon[(i + 2) % n] - polygon[i]) <= 0) {
            return false;
        }
    }
    return true;
}

/**
 * \brief check whether a given point lies within the convex hull
 *
 * @param hull  convex hull
 * @param p     point
 * @return true if the point lies inside the convex hull
 */
template <RangeOfVec<2> C>
bool isInside(const C& hull, const std::ranges::range_value_t<C>& p) {
    using E = std::ranges::range_value_t<C>;
    const std::size_t n = hull.size();
    if (n < 3) return false;

    // signed area of the triangle spanned by a and b
    auto cross2D = [](const E& a, const E& b) { return (a.x * b.y - a.y * b.x); };

    // look for a sign change between p and the current segment of the hull.
    // If it changes, then p lies on the outside.
    bool positive = cross2D(hull[1] - hull[0], p - hull[0]) >= 0;
    for (std::size_t i = 1; i < n; ++i) {
        if ((cross2D(hull[(i + 1) % n] - hull[i], p - hull[i]) >= 0) != positive) {
            return false;
        }
    }
    return true;
}

/**
 * \brief compute the area of a convex polygon
 *
 * @param polygon   points ordered counter-clockwise
 * @return area of polygon
 */
template <RangeOfVec<2> C>
double getArea(const C& polygon) {
    using E = std::ranges::range_value_t<C>;
    const std::size_t n = polygon.size();

    auto cross2D = [](const E& a, const E& b) { return (a.x * b.y - a.y * b.x); };
    double area = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        area += cross2D(polygon[(i + 1) % n] - polygon[i], polygon[(i + 2) % n] - polygon[i]);
    }
    area *= 0.5;
    return area;
}

/**
 * \brief compute the complex hull from a given set of 2D points using
 * the Monotone Chain algorithm, i.e. Andrew's convex hull algorithm
 * \see https://en.wikipedia.org/wiki/Convex_hull_algorithms#Algorithms
 *
 * @param points   set of 2D points
 * @return complex hull of input points
 */
template <RangeOfVec<2> C>
std::vector<std::ranges::range_value_t<C>> convexHull(const C& points) {
    using E = std::ranges::range_value_t<C>;
    // sort points according to x coordinate, if equal chose lower y coordinate
    std::vector<E> p = points;
    auto compare = [](const E& a, const E& b) {
        return (a.x < b.x) || ((a.x == b.x) && (a.y < b.y));
    };
    std::ranges::sort(p, compare);

    if (p.size() <= 3) {
        // trivial case
        return p;
    }

    // signed area of the triangle spanned by a and b.
    // Is used to determine the turn direction between a and b, i.e.
    // clockwise (cw, > 0), counter-clockwise (ccw, < 0) or co-linear (= 0)
    auto cross2D = [](const E& a, const E& b) { return (a.x * b.y - a.y * b.x); };

    const std::size_t n = points.size();
    std::vector<E> hull(2 * n);

    std::size_t k = 0;
    // build lower hull
    for (std::size_t i = 0; i < n; ++i) {
        while ((k > 1) && (cross2D(hull[k - 1] - hull[k - 2], p[i] - hull[k - 2]) <= 0)) {
            // last two points of the hull and p do not make a counter-clockwise turn
            // -> remove last hull point
            --k;
        }
        hull[k++] = p[i];
    }
    // build upper hull
    const std::size_t lastIndex = k + 1;
    for (int i = static_cast<int>(n) - 2; i >= 0; --i) {
        while ((k >= lastIndex) &&
               (cross2D(E(hull[k - 1] - hull[k - 2]), E(p[i] - hull[k - 2])) <= 0)) {
            // last two points of the hull and p do not make a counter-clockwise turn
            // -> remove last hull point
            --k;
        }
        hull[k++] = p[i];
    }
    // adjust hull size to k
    hull.resize(k - 1);
    return hull;
}

/**
 * \brief compute the complex hull from a given set of 2D points using
 * the Monotone Chain algorithm, i.e. Andrew's convex hull algorithm
 * \see https://en.wikipedia.org/wiki/Convex_hull_algorithms#Algorithms
 *
 * This version avoids unnecessary allocations by passing in the container @p hull where the result
 * is written. Note that the hull is of size 2*N.
 *
 * @param points   set of 2D points
 * @param hull     resulting convex hull of the input points, holding twice the number of points in
 *                 @p points
 * @return span of the actual hull pointing into @p hull
 */
template <size_t N, RangeOfVec<2> C, typename E = std::ranges::range_value_t<C>>
std::span<const E> convexHull(const C& points,
                              std::span<std::ranges::range_value_t<C>, 2 * N> hull) {
    if constexpr (N <= 3) {
        // trivial case
        std::ranges::copy_n(points, hull.begin, N);
        return hull.subspan(0, N);
    }

    // sort points according to x coordinate, if equal chose lower y coordinate
    std::array<E, N> p;
    std::ranges::copy(points, p.begin());
    auto compare = [](const E& a, const E& b) {
        return (a.x < b.x) || ((a.x == b.x) && (a.y < b.y));
    };
    std::ranges::sort(p, compare);

    // signed area of the triangle spanned by a and b.
    // Is used to determine the turn direction between a and b, i.e.
    // clockwise (cw, > 0), counter-clockwise (ccw, < 0) or co-linear (= 0)
    auto cross2D = [](const E& a, const E& b) { return (a.x * b.y - a.y * b.x); };

    std::size_t k = 0;
    // build lower hull
    for (std::size_t i = 0; i < N; ++i) {
        while ((k > 1) && (cross2D(hull[k - 1] - hull[k - 2], p[i] - hull[k - 2]) <= 0)) {
            // last two points of the hull and p do not make a counter-clockwise turn
            // -> remove last hull point
            --k;
        }
        hull[k++] = p[i];
    }
    // build upper hull
    const std::size_t lastIndex = k + 1;
    for (int i = static_cast<int>(N) - 2; i >= 0; --i) {
        while ((k >= lastIndex) &&
               (cross2D(E(hull[k - 1] - hull[k - 2]), E(p[i] - hull[k - 2])) <= 0)) {
            // last two points of the hull and p do not make a counter-clockwise turn
            // -> remove last hull point
            --k;
        }
        hull[k++] = p[i];
    }
    return hull.subspan(0, k - 1);
}

template <typename T>
std::vector<std::ranges::range_value_t<T>> convexHull(const T& /*points*/) {
    throw Exception(SourceContext{},
                    "util::complexHull() not implemented for nD points with n = {}",
                    util::extent<T>::value);
}

}  // namespace inviwo::util
