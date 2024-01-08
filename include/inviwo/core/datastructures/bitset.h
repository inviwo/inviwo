/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2024 Inviwo Foundation
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
#include <inviwo/core/io/serialization/serializable.h>

#include <span>

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

/**
 * \brief represents a bitset based on roaring bitmaps provided by the CRoaring library
 */
class IVW_CORE_API BitSet : public Serializable {
public:
    template <typename T>
    using iterator_category = typename std::iterator_traits<T>::iterator_category;
    template <class T>
    using is_iterator = util::is_detected<iterator_category, T>;

    class IVW_CORE_API BitSetIterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = uint32_t;
        using difference_type = std::ptrdiff_t;
        using pointer = const uint32_t*;
        using reference = const uint32_t&;

        BitSetIterator() = default;
        BitSetIterator(const BitSetIterator& rhs);
        BitSetIterator& operator=(const BitSetIterator& rhs);
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

        std::unique_ptr<RoaringIt> it_;
    };

    BitSet();

    BitSet(std::span<const uint32_t> span);

    BitSet(const std::vector<bool>& v);

    template <typename InputIt, class = std::enable_if_t<is_iterator<InputIt>::value>>
    BitSet(InputIt begin, InputIt end) : BitSet() {
        add(begin, end);
    }

    template <typename... Ts, class = std::enable_if_t<
                                  std::conjunction<std::is_convertible<Ts, uint32_t>...>::value>>
    BitSet(Ts&&... values) : BitSet() {
        add(values...);
    }

    BitSet(const BitSet& rhs);
    BitSet(BitSet&& rhs) noexcept;
    BitSet& operator=(const BitSet& rhs);
    BitSet& operator=(BitSet&& rhs) noexcept;
    ~BitSet();

    BitSetIterator begin() const;
    BitSetIterator end() const;

    /**
     * Return the number of elements the bitset holds
     *
     * \see size()
     */
    uint32_t cardinality() const;
    /**
     * Return the number of elements the bitset holds
     *
     * \see cardinality()
     */
    size_t size() const;

    bool empty() const;

    void clear();

    /**
     * Check whether this bitset is a subset of \p b
     */
    bool isSubsetOf(const BitSet& b) const;
    /**
     * Check whether this bitset is a strict subset of \p b
     */
    bool isStrictSubsetOf(const BitSet& b) const;

    /**
     * Replace the bitset with the contents of \p b, returns true if modified
     *
     * @return true if the bitset was modified that is \p this and \p b were different
     */
    bool set(const BitSet& b);

    void add(std::span<const uint32_t> span);
    void add(const std::vector<bool>& v);

    template <typename InputIt, class = typename std::enable_if_t<is_iterator<InputIt>::value>>
    void add(InputIt begin, InputIt end) {
        while (begin != end) {
            addSingle(*begin);
            ++begin;
        }
    }

    /**
     * Add value(s) \p values to the bitset
     *
     * @param values  one or more values
     */
    template <typename... Ts,
              typename = std::enable_if_t<std::conjunction_v<std::is_convertible<Ts, uint32_t>...>>>
    void add(Ts&&... values) {
        (addSingle(values), ...);
    }

    /**
     * Add value \p v to the bitset and return true on success
     *
     * @param v  value
     * @return true if \p v was added, false if it already existed in the bitset
     */
    bool addChecked(uint32_t v);

    /**
     * Adds all values from the open range [min,max)
     *
     * @param min   lower bound
     * @param max   upper bound (not included)
     */
    void addRange(uint32_t min, uint32_t max);

    /**
     * Adds all values from the closed range [min,max]
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
     * Flip the value \p v, that is add it to the bitset if not part of it, otherwise remove it
     *
     * @param v  value to flip
     */
    void flip(uint32_t v);

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
     * Compute the cardinality of the symmetric difference between this bitset and \p b
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
     * compute the union of multiple \p bitsets
     */
    static BitSet fastUnion(std::span<const BitSet*> bitsets);

    /**
     * Convert bitset to a std::vector holding only set elements. The output is ordered.
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

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    BitSet(const roaring::Roaring& roaring);
    BitSet(roaring::Roaring&& roaring);

    void addSingle(uint32_t value_);
    void addMany(size_t size, const uint32_t* data);

    std::unique_ptr<roaring::Roaring> roaring_;
};

}  // namespace inviwo
