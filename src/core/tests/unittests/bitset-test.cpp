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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <inviwo/core/datastructures/bitset.h>
#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/io/serialization/deserializer.h>

#include <tcb/span.hpp>

#include <vector>
#include <unordered_set>
#include <algorithm>
#include <random>
#include <sstream>

namespace inviwo {

std::vector<std::uint32_t> getIndices(const std::size_t count) {
    std::mt19937 gen;
    gen.seed(1);
    std::uniform_int_distribution<std::uint32_t> distrib(0, 4096);

    std::unordered_set<std::uint32_t> indices;
    while (indices.size() < count) {
        indices.insert(distrib(gen));
    }
    std::vector<std::uint32_t> v(indices.begin(), indices.end());
    std::shuffle(v.begin(), v.end(), std::default_random_engine());
    return v;
}

TEST(bitset, constructors) {
    BitSet();
    BitSet(5);
    BitSet(1, 2);
    BitSet(2, 3, 5);

    std::vector<std::uint32_t> indices = getIndices(20);
    BitSet{indices};
    BitSet(util::span<std::uint32_t>(indices));
    BitSet(indices.begin(), indices.end());

    EXPECT_TRUE(true);
}

TEST(bitset, empty) {
    BitSet b;
    EXPECT_TRUE(b.empty());

    b.add(5);
    EXPECT_FALSE(b.empty());

    b.remove(5);
    EXPECT_TRUE(b.empty());
}

TEST(bitset, cardinality) {
    BitSet b;
    EXPECT_EQ(0, b.cardinality());

    b.add(5);
    EXPECT_EQ(1, b.cardinality());

    b.add(5);
    EXPECT_EQ(1, b.cardinality());
}

TEST(bitset, add_checked) {
    BitSet b;
    EXPECT_TRUE(b.addChecked(1));
    EXPECT_TRUE(b.addChecked(2));
    EXPECT_FALSE(b.addChecked(1));
}

TEST(bitset, add_singleElement) {
    BitSet b;

    b.add(5);
    b.add(2);

    EXPECT_EQ(2, b.cardinality());
}

TEST(bitset, add_iterators) {
    std::vector<std::uint32_t> indices = getIndices(20);

    BitSet b;
    b.add(indices.begin(), indices.end());

    EXPECT_EQ(20, b.cardinality());
}

TEST(bitset, add_boolVec) {
    auto boolVec = [](const auto& indices) {
        std::vector<bool> v(*std::max_element(indices.begin(), indices.end()) + 1, false);
        for (auto i : indices) {
            v[i] = true;
        }
        return v;
    };

    auto test = [&](const std::vector<std::uint32_t>& indices, bool pushFalse) {
        std::vector<bool> v = boolVec(indices);
        if (pushFalse) {
            v.push_back(false);
        }
        BitSet b(v);
        EXPECT_EQ(indices.size(), b.cardinality());
        for (auto index : indices) {
            EXPECT_TRUE(b.contains(index));
        }
    };

    test({0}, false);
    test({0, 2}, false);
    test({0, 2}, true);
    test({1, 2}, true);
    test({1, 2, 4, 6, 7, 8}, false);
    test({1, 2, 4, 6, 7, 8}, true);
}

TEST(bitset, contains) {
    BitSet b;

    b.add(5);
    b.add(2);
    EXPECT_TRUE(b.contains(5));
    EXPECT_TRUE(b.contains(2));
    EXPECT_FALSE(b.contains(42));

    std::vector<std::uint32_t> indices = getIndices(20);
    BitSet b1(indices);
    for (auto index : indices) {
        EXPECT_TRUE(b1.contains(index));
    }

    BitSet b2(indices.begin(), indices.end());
    for (auto index : indices) {
        EXPECT_TRUE(b2.contains(index));
    }
}

TEST(bitset, minmax) {
    std::vector<std::uint32_t> indices = getIndices(20);

    auto b = BitSet(indices.begin(), indices.end());

    EXPECT_EQ(*std::min_element(indices.begin(), indices.end()), b.min());
    EXPECT_EQ(*std::max_element(indices.begin(), indices.end()), b.max());
}

TEST(bitset, serialization) {
    std::vector<std::uint32_t> indices = getIndices(20);

    std::stringstream ss;
    Serializer serializer("");

    auto b = BitSet(indices.begin(), indices.end());
    b.serialize(serializer);
    serializer.writeFile(ss);

    BitSet result;
    Deserializer deserializer(ss, "");
    result.deserialize(deserializer);
    EXPECT_EQ(b, result);
}

TEST(bitset, binarySerialization) {
    std::vector<std::uint32_t> indices = getIndices(20);

    std::stringstream ss;

    auto b = BitSet(indices.begin(), indices.end());
    b.writeData(ss);

    BitSet result;
    result.readData(ss);
    EXPECT_EQ(b, result);
}

TEST(bitset, iterators) {
    std::vector<std::uint32_t> indices = getIndices(5);
    auto b = BitSet(indices.begin(), indices.end());

    for (auto index : b) {
        EXPECT_TRUE(util::contains(indices, index));
    }

    std::vector<std::uint32_t> vec(b.begin(), b.end());
    EXPECT_EQ(b.size(), vec.size());
    for (auto index : vec) {
        EXPECT_TRUE(util::contains(indices, index));
    }
}

}  // namespace inviwo
