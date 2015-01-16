/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_POLYGON_H
#define IVW_POLYGON_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/geometry/edge.h>

namespace inviwo {

/**
 * \class Polygon
 * \brief Create a Polygon which consists of points.
 *
 * A Polygon consists of a specific number of vertices, or indices to vertices.
 * When polygon contains vertices, adjacent vertices are considered to be connected with lines to form a edge.
 * First and last vertex are also considered to be connected, to form a closed chain.
 */

template<typename T>
class Polygon  {

public:
    Polygon(size_t size) : size_(size) {
        list_ = new T[size_];
    }

    Polygon(const Polygon& rhs) : size_(rhs.size()) {
        list_ = new T[size_];

        for (size_t i=0; i < size(); ++i)
            list_[i] = rhs.get(i);
    }

    virtual ~Polygon() {
        delete[] list_;
    }

    T& at(size_t i) {
        return list_[i];
    }

    T get(size_t i) const {
        return list_[i];
    }

    size_t size() const {
        return size_;
    }

    const T* values() const {
        return list_;
    }

    T* values() {
        return list_;
    }

private:
    T* list_;
    size_t size_;
};

} // namespace

#endif // IVW_POLYGON_H