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

#ifndef IVW_CONVEXHULL_H
#define IVW_CONVEXHULL_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/util/exception.h>

#include <vector>
#include <algorithm>

namespace inviwo {

namespace util {

/**
 * \brief check whether the given polygon is convex
 *
 * @param polygon   polygon consisting of points
 * @return true if the polygon is convex, false otherwise
 */
template <class T, typename std::enable_if<util::rank<T>::value == 1 && util::extent<T>::value == 2,
                                           int>::type = 0>
bool isConvex(const std::vector<T> &polygon) {
    const std::size_t n = polygon.size();
    if (n < 3) return true;

    auto cross2D = [](const T &a, const T &b) { return (a.x * b.y - a.y * b.x); };

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
template <class T, typename std::enable_if<util::rank<T>::value == 1 && util::extent<T>::value == 2,
                                           int>::type = 0>
bool isInside(const std::vector<T> &hull, const T &p) {
    const std::size_t n = hull.size();
    if (n < 3) return false;

    // signed area of the triangle spanned by a and b
    auto cross2D = [](const T &a, const T &b) { return (a.x * b.y - a.y * b.x); };

    // look for a sign change between p and the current segment of the hull.
    // If it changes, then p lies on the outside.
    bool positive = cross2D(hull[1] - hull[0], p - hull[0]) > 0.0;
    for (std::size_t i = 1; i < n; ++i) {
        if ((cross2D(hull[(i + 1) % n] - hull[i], p - hull[i]) > 0.0) != positive) {
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
template <class T, typename std::enable_if<util::rank<T>::value == 1 && util::extent<T>::value == 2,
                                           int>::type = 0>
double getArea(const std::vector<T> &polygon) {
    const std::size_t n = polygon.size();

    auto cross2D = [](const T &a, const T &b) { return (a.x * b.y - a.y * b.x); };
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
template <class T, typename std::enable_if<util::rank<T>::value == 1 && util::extent<T>::value == 2,
                                           int>::type = 0>
std::vector<T> convexHull(const std::vector<T> &points) {
    // sort points according to x coordinate, if equal chose lower y coordinate
    std::vector<T> p = points;
    auto compare = [](const T &a, const T &b) {
        return (a.x < b.x) || ((a.x == b.x) && (a.y < b.y));
    };
    std::sort(p.begin(), p.end(), compare);

    if (p.size() <= 3) {
        // trivial case
        return p;
    }

    // signed area of the triangle spanned by a and b.
    // Is used to determine the turn direction between a and b, i.e.
    // clockwise (cw, > 0), counter-clockwise (ccw, < 0) or co-linear (= 0)
    auto cross2D = [](const T &a, const T &b) { return (a.x * b.y - a.y * b.x); };

    const std::size_t n = points.size();
    std::vector<T> hull(2 * n);

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
               (cross2D(T(hull[k - 1] - hull[k - 2]), T(p[i] - hull[k - 2])) <= 0)) {
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

template <class T, typename std::enable_if<util::rank<T>::value == 1 && util::extent<T>::value != 2,
                                           int>::type = 0>
std::vector<T> convexHull(const std::vector<T> & /*points*/) {
    std::ostringstream message;
    message << "util::complexHull() not implemented for nD points with n = "
            << util::extent<T>::value;
    throw Exception(message.str());
}

}  // namespace util

}  // namespace inviwo

#endif  // IVW_CONVEXHULL2D_H
