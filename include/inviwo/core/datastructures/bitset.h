/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/detected.h>

#include <memory>
#include <vector>
#include <array>
#include <iterator>
#include <type_traits>
#include <istream>
#include <ostream>

namespace roaring {
class Roaring;
class RoaringSetBitForwardIterator;
}  // namespace roaring

namespace inviwo {

namespace detail {

template <typename...>
using void_t = void;

template <class T, class = void>
struct is_iterator : std::false_type {};

template <class T>
struct is_iterator<T, void_t<typename std::iterator_traits<T>::iterator_category>>
    : std::true_type {};

}  // namespace detail

/**
 * \brief represents a bitset based on roaring bitmaps provided by the CRoaring library
 */
class IVW_CORE_API BitSet {
public:
    class BitSetIterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = uint32_t;
        using difference_type = uint32_t;
        using pointer = const uint32_t*;
        using reference = const uint32_t&;

        BitSetIterator() = default;
        ~BitSetIterator();

        BitSetIterator& operator++();
        BitSetIterator operator++(int);
        BitSetIterator& operator--();
        BitSetIterator operator--(int);

        value_type operator*() const;

        bool operator==(const BitSetIterator& rhs) const;
        bool operator!=(const BitSetIterator& rhs) const;

    private:
        using RoaringIt = roaring::RoaringSetBitForwardIterator;

        friend BitSet;
        BitSetIterator(const BitSet& b, bool exhausted = false);
        BitSetIterator(const RoaringIt& it);
        BitSetIterator(const BitSetIterator& rhs);
        BitSetIterator& operator=(const BitSetIterator& rhs);

        std::unique_ptr<RoaringIt> it_;
    };

    BitSet();

    BitSet(const std::vector<uint32_t>& values);

    template <size_t N>
    BitSet(const std::array<uint32_t, N>& values) : BitSet() {
        addMany(values.size(), values.data());
    }

    template <typename InputIt,
              class = typename std::enable_if_t<detail::is_iterator<InputIt>::value>>
    explicit BitSet(InputIt begin, InputIt end) : BitSet() {
        add(begin, end);
    }

    template <typename... uint32_ts>
    explicit BitSet(uint32_t val, uint32_ts&&... values) : BitSet() {
        add(val, values...);
    }

    BitSet(const BitSet& rhs);
    BitSet(BitSet&& rhs) noexcept;
    BitSet& operator=(const BitSet& rhs);
    BitSet& operator=(BitSet&& rhs) noexcept;
    ~BitSet() = default;

    BitSetIterator begin() const;
    BitSetIterator end() const;

    /**
     * return the number of elements the bitset holds
     */
    uint32_t cardinality() const;

    bool empty() const;

    void clear();

    /**
     * check whether this bitset is a subset of \p b
     */
    bool isSubsetOf(const BitSet& b) const;
    /**
     * check whether this bitset is a strict subset of \p b
     */
    bool isStrictSubsetOf(const BitSet& b) const;

    /**
     * Add value \p v to the bitset
     *
     * @param v  value
     */
    void add(uint32_t v);

    void add(const std::vector<uint32_t>& values);

    template <size_t N>
    void add(const std::array<uint32_t, N>& values) {
        addMany(values.size(), values.data());
    }

    template <typename InputIt,
              class = typename std::enable_if_t<detail::is_iterator<InputIt>::value>>
    void add(InputIt begin, InputIt end) {
        while (begin != end) {
            add(*begin);
            ++begin;
        }
    }

    template <typename... Ts>
    void add(uint32_t val, Ts&&... values) {
        add(val);
        add(values...);
    }

    /**
     * Add value \p v to the bitset and return true on success
     *
     * @param v  value
     * @return true if \p v was added, false if it already existed in the bitset
     */
    bool addChecked(uint32_t v);

    /**
     * adds all values from the open range [min,max)
     *
     * @param min   lower bound
     * @param max   upper bound (not included)
     */
    void addRange(uint32_t min, uint32_t max);

    /**
     * adds all values from the closed range [min,max]
     *
     * @param min   lower bound
     * @param max   upper bound (included)
     */
    void addRangeClosed(uint32_t min, uint32_t max);

    /**
     * Remove value \p v to the bitset
     *
     * @param v  value
     */
    void remove(uint32_t v);

    /**
     * Remove value \p v to the bitset and return true on success
     *
     * @param v  value
     * @return true if \p v was removed, false if it did not exist in the bitset
     */
    bool removeChecked(uint32_t v);

    /**
     * Return the largest value
     *
     * @return largest value in the bitset, 0 if empty
     */
    uint32_t max() const;
    /**
     * Return the smallest value
     *
     * @return smallest value in the bitset, UINT32_MAX if empty
     */
    uint32_t min() const;

    /**
     * Checks whether \p v is part of the bitset
     *
     * @param v  value to check
     * @return true if \p v exists
     */
    bool contains(uint32_t v) const;

    /**
     * Checks whether the open range [\p min, \p max) is part of the bitset
     *
     * @param min  lower bound
     * @param max  upper bound
     * @return true if [\p min, \p max) exists
     */
    bool containsRange(uint32_t min, uint32_t max) const;

    /**
     * Flip all bits in the open range [\p min, \p max)
     */
    void flipRange(uint32_t min, uint32_t max);

    /**
     * Return the number of values that are less or equal to \p v
     */
    size_t rank(uint32_t v) const;

    /**
     * Compare whether this bitset and \p b are identical
     */
    bool operator==(const BitSet& b) const;

    /**
     * Compare whether this bitset and \p b are different
     */
    bool operator!=(const BitSet& b) const;

    /**
     * Checks whether this bitset and \p b intersect
     */
    bool intersect(const BitSet& b) const;

    /**
     * Compute the cardinality of the union between this bitset and \p b
     */
    size_t orCardinality(const BitSet& b) const;
    /**
     * Compute the cardinality of the intersection between this bitset and \p b
     */
    size_t andCardinality(const BitSet& b) const;
    /**
     * Compute the cardinality of the difference between this bitset and \p b
     */
    size_t andNotCardinality(const BitSet& b) const;
    /**
     * Compute the cardinality of the symmetric differencebetween this bitset and \p b
     */
    size_t xorCardinality(const BitSet& b) const;

    /**
     * Computes the Jaccard index between this bitset and \p b. This distance is also known as the
     * Tanimoto distance, or the Jaccard similarity coefficient.
     *
     * The Jaccard index is undefined if both bitsets are empty.
     */
    double jaccardIndex(const BitSet& b) const;

    /**
     * Intersection of this bitset and \p b
     */
    BitSet& operator&=(const BitSet& b);
    /**
     * Intersection of this bitset and \p b
     */
    BitSet operator&(const BitSet& b) const;

    /**
     * Difference of this bitset and \p b
     */
    BitSet& operator-=(const BitSet& b);
    /**
     * Difference of this bitset and \p b
     */
    BitSet operator-(const BitSet& b) const;

    /**
     * Union of this bitset and \p b
     */
    BitSet& operator|=(const BitSet& b);
    /**
     * Union of this bitset and \p b
     */
    BitSet operator|(const BitSet& b) const;

    /**
     * Symmetric union of this bitset and \p b
     */
    BitSet& operator^=(const BitSet& b);
    /**
     * Symmetric union of this bitset and \p b
     */
    BitSet operator^(const BitSet& b) const;

    /**
     * Convert bitset to a std::vector holding only set elements
     */
    std::vector<uint32_t> toVector() const;
    /**
     * Create a string from the bitset
     */
    std::string toString() const;

    /**
     * Return the number of bytes required to serialize the bitset
     *
     * \see optimize(), shrinkToFit()
     */
    size_t getSizeInBytes() const;

    /**
     * Write this bitset to stream \p os. The first four byte value contains the size of the bitset
     * in bytes followed by the binary encoding.
     */
    void writeData(std::ostream& os) const;

    /**
     * Read a BitSet from stream \p is. The first four byte value contains the size of the bitset in
     * bytes.
     *
     * @throw Exception
     */
    void readData(std::istream& is);

    friend std::ostream& operator<<(std::ostream& os, const BitSet& b) {
        os << b.toString();
        return os;
    }

    /**
     * Optimize the bitset representation using run-length encoding based on space efficiency.
     * Calling shrinkToFit() might result in further memory savings.
     */
    void optimize();

    /**
     * Enforce removing the run-length encoding of the bitset even though it might be more
     * efficient.
     *
     * \see optimize()
     */
    void removeRLECompression();

    /**
     * Reallocates the memory needed.
     *
     * @return number of bytes saved
     */
    size_t shrinkToFit();

private:
    void addMany(size_t size, const uint32_t* data);

    struct IVW_CORE_API RoaringDeleter {
        void operator()(roaring::Roaring* ptr) const;
    };

    std::unique_ptr<roaring::Roaring, RoaringDeleter> roaring_;
};

}  // namespace inviwo
