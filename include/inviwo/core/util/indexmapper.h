/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#ifndef IVW_INDEXMAPPER_H
#define IVW_INDEXMAPPER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/glm.h>

namespace inviwo {
namespace util {

template <size_t N, typename IndexType = size_t>
struct IndexMapper {};

template <typename IndexType>
struct IndexMapper<2, IndexType> {
    constexpr IndexMapper(const Vector<2, IndexType> &dim) : dimx(dim.x){};
    constexpr IndexType operator()(const IndexType x, const IndexType y) const noexcept {
        return x + y * dimx;
    }
    constexpr IndexType operator()(const Vector<2, IndexType> &pos) const noexcept {
        return pos.x + pos.y * dimx;
    }
    constexpr Vector<2, IndexType> operator()(const IndexType index) const noexcept {
        return Vector<2, IndexType>(index % dimx, index / dimx);
    }

private:
    const IndexType dimx;
};

template <typename IndexType>
struct IndexMapper<3, IndexType> {
    constexpr IndexMapper(const Vector<3, IndexType> &dim) noexcept
        : dimx(dim.x), dimxy(dim.x * dim.y){};
    constexpr IndexType operator()(const IndexType x, const IndexType y, const IndexType z) const
        noexcept {
        return x + y * dimx + z * dimxy;
    }
    constexpr IndexType operator()(const Vector<3, IndexType> &pos) const noexcept {
        return pos.x + pos.y * dimx + pos.z * dimxy;
    }
    constexpr Vector<3, IndexType> operator()(const IndexType index) const noexcept {
        return Vector<3, IndexType>((index % dimxy) % dimx, (index % dimxy) / dimx, index / dimxy);
    }

private:
    const IndexType dimx;
    const IndexType dimxy;
};

using IndexMapper2D = IndexMapper<2, size_t>;
using IndexMapper3D = IndexMapper<3, size_t>;

template <size_t N, typename IndexType = size_t>
auto makeIndexMapper(const Vector<N, IndexType> &dim) {
    return IndexMapper<N, IndexType>(dim);
}

}  // namespace util
}  // namespace inviwo

#endif  // IVW_INDEXMAPPER_H
