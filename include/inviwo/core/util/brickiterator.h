/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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

#include <inviwo/core/util/indexmapper.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/glmvec.h>

#include <iterator>
#include <algorithm>

namespace inviwo {

namespace util {

/**
 * \class BrickIterator
 * \brief An iterator providing access to a subregion, or brick, within linearized 3D data.
 *
 * This iterator provides access to and iterates over a subregion within 3D data. The 3D data
 * is assumed to be contiguous, i.e. linearized, in x, then y, then z similar to `IndexMapper3D`.
 * Initially, the iterator points to the first voxel of the brick. Iteration takes place in x
 * direction, followed by y and z, respectively.
 *
 * `BrickIterator::end()` provides the matching end iterator
 *
 * \see util::IndexMapper3D
 *
 * Example:
 * \code{.cpp}
 * std::vector<int> data(27); // linearized data of a 3x3x3 volume
 *
 * // create a brick iterator starting at (1,1,1) with an extent of (2,2,2)
 * auto it = util::BrickIterator{data.begin(), size3_t{3, 3, 3},
 *                               size3_t{1, 1, 1}, size3_t{2, 2, 2}};
 *
 * // copy the data elements of the brick iterator into a std::vector,
 * // the vector will contain the corresponding 8 voxels
 * std::vector<int> block(it, it.end());
 * \endcode
 *
 * Here, a single voxel at position (2,0,0) of a VolumeRAM is extracted.
 * \code{.cpp}
 * auto volume = VolumeRAMPrecision<float>(size3_t{3, 3, 3});
 *
 * int value = *util::BrickIterator{volume.getDataTyped(), volume.getDimensions(),
 *                                  size3_t{2, 0, 0}, size3_t{1, 1, 1}};
 * \endcode
 *
 * Modify a subregion within a volume.
 * \code{.cpp}
 * std::vector<int> data(5 * 4 * 6);
 *
 * auto it = util::BrickIterator{data.begin(), size3_t{5, 4, 6},
 *                               size3_t{3, 1, 2}, size3_t{2, 3, 3}};
 * for (auto& elem : it) {
 *   elem *= 2;
 * }
 * \endcode
 */
template <typename Iter>
class BrickIterator {
public:
    using base_category = typename std::iterator_traits<Iter>::iterator_category;

    using difference_type = typename std::iterator_traits<Iter>::difference_type;
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = typename std::iterator_traits<Iter>::value_type;
    using pointer = typename std::iterator_traits<Iter>::pointer;
    using reference = typename std::iterator_traits<Iter>::reference;

    BrickIterator() = default;
    /**
     * Creates an brick iterator given an iterator to an array containing linearized volume data
     * and its corresponding volume dimensions.
     *
     * @param iterator  iterator pointing to the begin of the linearized source data
     * @param dims      dimensions of the source volume
     * @param offset    starting position of the brick
     * @param extent    extent of the brick, i.e. the region the iterator should iterate over
     */
    BrickIterator(Iter iterator, size3_t dims, size3_t offset, size3_t extent)
        : current_{0, 0, 0}, start_{offset}, extent_{extent}, im_{dims}, iterator_{iterator} {

        IVW_ASSERT(glm::all(glm::lessThanEqual(start_ + extent_, dims)), "Invalid extents");
    }

    BrickIterator& operator++() {
        ++current_.x;
        if (current_.x == extent_.x) {
            current_.x = 0;
            ++current_.y;
        }
        if (current_.y == extent_.y) {
            current_.y = 0;
            ++current_.z;
        }
        return *this;
    }
    BrickIterator operator++(int) {
        auto it = this;
        operator++();
        return it;
    }

    BrickIterator& operator--() {
        if (current_.x == 0) {
            current_.x = extent_.x - 1;
            if (current_.y == 0) {
                current_.y = extent_.y - 1;
                --current_.z;
            } else {
                --current_.y;
            }
        } else {
            --current_.x;
        }

        return *this;
    }
    BrickIterator operator--(int) {
        auto it = this;
        operator--();
        return it;
    }

    reference operator*() const { return (iterator_ + im_(start_ + current_)).operator*(); }
    pointer operator->() const { return (iterator_ + im_(start_ + current_)).operator->(); }

    bool operator==(const BrickIterator& rhs) const { return current_ == rhs.current_; }
    bool operator!=(const BrickIterator& rhs) const { return current_ != rhs.current_; }

    const Iter& base() const { return iterator_ + im_(start_ + current_); }
    Iter& base() { return iterator_ + im_(start_ + current_); }

    size3_t globalPos() const { return start_ + current_; }
    size3_t blockPos() const { return current_; }

    BrickIterator begin() const { return {*this}; }

    BrickIterator end() const {
        auto it{*this};
        it.current_ = size3_t{0, 0, extent_.z};
        return it;
    }

private:
    size3_t current_;
    size3_t start_;
    size3_t extent_;
    util::IndexMapper3D im_;
    Iter iterator_;
};

}  // namespace util

}  // namespace inviwo
