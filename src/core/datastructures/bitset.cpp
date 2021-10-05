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

#include <inviwo/core/datastructures/bitset.h>
#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/io/serialization/deserializer.h>
#include <inviwo/core/algorithm/base64.h>

#include <warn/push>
#include <warn/ignore/all>
#include <roaring.hh>
#include <warn/pop>

namespace inviwo {

BitSet::BitSetIterator::BitSetIterator(const BitSetIterator& rhs)
    : it_(std::make_unique<RoaringIt>(*rhs.it_)) {}

auto BitSet::BitSetIterator::operator=(const BitSetIterator& rhs) -> BitSetIterator& {
    if (this != &rhs) {
        *it_ = *rhs.it_;
    }
    return *this;
}

BitSet::BitSetIterator::~BitSetIterator() = default;

BitSet::BitSetIterator::BitSetIterator(const BitSet& b, bool exhausted)
    : it_(std::make_unique<RoaringIt>(*b.roaring_, exhausted)) {}

BitSet::BitSetIterator::BitSetIterator(const RoaringIt& it)
    : it_(std::make_unique<RoaringIt>(it)) {}

BitSet::BitSetIterator& BitSet::BitSetIterator::operator++() {
    it_->operator++();
    return *this;
}

BitSet::BitSetIterator BitSet::BitSetIterator::operator++(int) {
    BitSet::BitSetIterator it(*it_);
    it_->operator++();
    return it;
}

BitSet::BitSetIterator& BitSet::BitSetIterator::operator--() {
    it_->operator--();
    return *this;
}

BitSet::BitSetIterator BitSet::BitSetIterator::operator--(int) {
    BitSetIterator it(*it_);
    it_->operator--();
    return it;
}

auto BitSet::BitSetIterator::operator*() const -> value_type { return **it_; }

bool BitSet::BitSetIterator::operator==(const BitSetIterator& rhs) const {
    return it_->operator==(*rhs.it_);
}

bool BitSet::BitSetIterator::operator!=(const BitSetIterator& rhs) const {
    return it_->operator!=(*rhs.it_);
}

BitSet::BitSet() : roaring_(std::make_unique<roaring::Roaring>()) {}

BitSet::BitSet(util::span<const uint32_t> span) : BitSet() { addMany(span.size(), span.data()); }

BitSet::BitSet(const roaring::Roaring& roaring)
    : roaring_(std::make_unique<roaring::Roaring>(roaring)) {}

BitSet::BitSet(roaring::Roaring&& roaring)
    : roaring_(std::make_unique<roaring::Roaring>(std::move(roaring))) {}

BitSet::BitSet(const BitSet& rhs) : roaring_(std::make_unique<roaring::Roaring>(*rhs.roaring_)) {}

BitSet::BitSet(BitSet&& rhs) noexcept : roaring_(std::move(rhs.roaring_)) {}

BitSet::~BitSet() = default;

BitSet& BitSet::operator=(const BitSet& rhs) {
    if (this != &rhs) {
        *roaring_ = *rhs.roaring_;
    }
    return *this;
}

BitSet& BitSet::operator=(BitSet&& rhs) noexcept {
    roaring_ = std::move(rhs.roaring_);
    return *this;
}

auto BitSet::begin() const -> BitSetIterator { return BitSetIterator(*this, false); }

auto BitSet::end() const -> BitSetIterator { return BitSetIterator(*this, true); }

uint32_t BitSet::cardinality() const { return static_cast<uint32_t>(roaring_->cardinality()); }

size_t BitSet::size() const { return static_cast<size_t>(roaring_->cardinality()); }

bool BitSet::empty() const { return roaring_->isEmpty(); }

void BitSet::clear() { roaring::api::roaring_bitmap_clear(&roaring_->roaring); }

bool BitSet::isSubsetOf(const BitSet& b) const { return roaring_->isSubset(*(b.roaring_)); }

bool BitSet::isStrictSubsetOf(const BitSet& b) const {
    return roaring_->isStrictSubset(*(b.roaring_));
}

void BitSet::add(util::span<const uint32_t> span) { addMany(span.size(), span.data()); }

bool BitSet::addChecked(uint32_t v) { return roaring_->addChecked(v); }

void BitSet::addRange(uint32_t min, uint32_t max) { roaring_->addRange(min, max); }

void BitSet::addRangeClosed(uint32_t min, uint32_t max) {
    roaring::api::roaring_bitmap_add_range_closed(&roaring_->roaring, min, max);
}

void BitSet::remove(uint32_t v) { roaring_->remove(v); }

bool BitSet::removeChecked(uint32_t v) { return roaring_->removeChecked(v); }

uint32_t BitSet::max() const { return roaring_->maximum(); }

uint32_t BitSet::min() const { return roaring_->minimum(); }

bool BitSet::contains(uint32_t v) const { return roaring_->contains(v); }

bool BitSet::containsRange(uint32_t min, uint32_t max) const {
    return roaring_->containsRange(min, max);
}

void BitSet::flip(uint32_t v) { roaring_->flip(v, v + 1); }

void BitSet::flipRange(uint32_t min, uint32_t max) { roaring_->flip(min, max); }

size_t BitSet::rank(uint32_t v) const { return roaring_->rank(v); }

bool BitSet::operator==(const BitSet& b) const { return roaring_->operator==(*b.roaring_); }

bool BitSet::operator!=(const BitSet& b) const { return !operator==(b); }

bool BitSet::intersect(const BitSet& b) const { return roaring_->intersect(*(b.roaring_)); }

size_t BitSet::orCardinality(const BitSet& b) const {
    return roaring_->and_cardinality(*(b.roaring_));
}

size_t BitSet::andCardinality(const BitSet& b) const {
    return roaring_->and_cardinality(*(b.roaring_));
}

size_t BitSet::andNotCardinality(const BitSet& b) const {
    return roaring_->and_cardinality(*(b.roaring_));
}

size_t BitSet::xorCardinality(const BitSet& b) const {
    return roaring_->and_cardinality(*(b.roaring_));
}

double BitSet::jaccardIndex(const BitSet& b) const {
    return roaring_->jaccard_index(*(b.roaring_));
}

BitSet& BitSet::operator&=(const BitSet& b) {
    roaring_->operator&=(*(b.roaring_));
    return *this;
}

BitSet BitSet::operator&(const BitSet& b) const {
    BitSet result;
    *result.roaring_ = roaring_->operator&(*(b.roaring_));
    return result;
}

BitSet& BitSet::operator-=(const BitSet& b) {
    roaring_->operator-=(*(b.roaring_));
    return *this;
}

BitSet BitSet::operator-(const BitSet& b) const {
    BitSet result;
    *result.roaring_ = roaring_->operator-(*(b.roaring_));
    return result;
}

BitSet& BitSet::operator|=(const BitSet& b) {
    roaring_->operator|=(*(b.roaring_));
    return *this;
}

BitSet BitSet::operator|(const BitSet& b) const {
    BitSet result;
    *result.roaring_ = roaring_->operator|(*(b.roaring_));
    return result;
}

BitSet& BitSet::operator^=(const BitSet& b) {
    roaring_->operator^=(*(b.roaring_));
    return *this;
}

BitSet BitSet::operator^(const BitSet& b) const {
    BitSet result;
    *result.roaring_ = roaring_->operator^(*(b.roaring_));
    return result;
}

BitSet BitSet::fastUnion(util::span<const BitSet*> bitsets) {
    using namespace roaring;

    std::vector<const Roaring*> inputs;
    for (auto b : bitsets) {
        inputs.push_back(b->roaring_.get());
    }

    return BitSet(Roaring::fastunion(inputs.size(), inputs.data()));
}

std::vector<uint32_t> BitSet::toVector() const {
    std::vector<uint32_t> v(cardinality());
    roaring_->toUint32Array(v.data());
    return v;
}

std::string BitSet::toString() const { return roaring_->toString(); }

size_t BitSet::getSizeInBytes() const { return roaring_->getSizeInBytes(true); }

void BitSet::writeData(std::ostream& os) const {
    std::vector<char> buf(getSizeInBytes());
    const size_t numBytes = roaring_->write(buf.data(), true);
    os << static_cast<uint32_t>(numBytes);
    os.write(buf.data(), numBytes);
}

void BitSet::readData(std::istream& is) {
    try {
        uint32_t numBytes = 0u;
        is >> numBytes;
        std::vector<char> buf(numBytes);
        is.read(buf.data(), numBytes);
        *roaring_ = roaring::Roaring::read(buf.data(), true);
    } catch (std::runtime_error) {
        throw Exception("Error reading BitSet", IVW_CONTEXT);
    }
}

void BitSet::optimize() { roaring_->runOptimize(); }

void BitSet::removeRLECompression() { roaring_->removeRunCompression(); }

size_t BitSet::shrinkToFit() { return roaring_->shrinkToFit(); }

void BitSet::serialize(Serializer& s) const {
    std::vector<char> buf(getSizeInBytes());
    roaring_->write(buf.data(), true);

    s.serialize("bitset", util::base64_encode(buf));
}

void BitSet::deserialize(Deserializer& d) {
    std::string str;
    d.deserialize("bitset", str);

    util::span<char> buf = util::base64_decode(str);
    *roaring_ = roaring::Roaring::read(buf.data(), true);
}

void BitSet::addSingle(uint32_t v) { roaring_->add(v); }

void BitSet::addMany(size_t size, const uint32_t* data) { roaring_->addMany(size, data); }

}  // namespace inviwo
