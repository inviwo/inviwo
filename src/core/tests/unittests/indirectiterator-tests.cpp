/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <vector>
#include <memory>
#include <type_traits>

#include <inviwo/core/util/indirectiterator.h>

namespace inviwo {

TEST(IndirectIteratorTest, Pair) {
    std::vector<int> ref;
    std::vector<std::unique_ptr<int>> vec;
    for (int i = 0; i < 5; ++i) {
        ref.push_back(i);
        vec.push_back(std::make_unique<int>(i));
    }

    const auto& cvec = vec;

    {
        auto b = util::makeIndirectIterator(vec.begin());
        auto e = util::makeIndirectIterator(vec.end());
        auto rb = ref.begin();
        for (; b != e; ++b, ++rb) {
            EXPECT_EQ(*b, *rb);
        }
    }

    {
        auto b = util::makeIndirectIterator(vec.begin());
        auto e = util::makeIndirectIterator(vec.end());
        for (; b != e; ++b) {
            *b += 10;
        }
    }
    {
        auto b = util::makeIndirectIterator(vec.begin());
        auto e = util::makeIndirectIterator(vec.end());
        auto rb = ref.begin();
        for (; b != e; ++b, ++rb) {
            EXPECT_EQ(*b, *rb + 10);
        }
    }

    {
        auto b = util::makeIndirectIterator(cvec.begin());
        auto e = util::makeIndirectIterator(cvec.end());
        auto rb = ref.begin();
        for (; b != e; ++b, ++rb) {
            EXPECT_EQ(*b, *rb + 10);
        }
    }

    {
        auto b = util::makeIndirectIterator<false>(cvec.begin());
        auto e = util::makeIndirectIterator<false>(cvec.end());
        for (; b != e; ++b) {
            *b += 10;
        }
    }

    {
        auto b = util::makeIndirectIterator(cvec.begin());
        auto e = util::makeIndirectIterator(cvec.end());
        auto rb = ref.begin();
        for (; b != e; ++b, ++rb) {
            EXPECT_EQ(*b, *rb + 20);
        }
    }

    {
        auto b = util::makeIndirectIterator(vec.begin());
        static_assert(std::is_assignable<decltype(*b), int>::value, "");
    }
    {
        auto b = util::makeIndirectIterator(cvec.begin());
        static_assert(!std::is_assignable<decltype(*b), int>::value, "");
    }
    {
        auto b = util::makeIndirectIterator(vec.cbegin());
        static_assert(!std::is_assignable<decltype(*b), int>::value, "");
    }
    {
        auto b = util::makeIndirectIterator(cvec.cbegin());
        static_assert(!std::is_assignable<decltype(*b), int>::value, "");
    }
    {
        auto b = util::makeIndirectIterator<false>(vec.begin());
        static_assert(std::is_assignable<decltype(*b), int>::value, "");
    }
    {
        auto b = util::makeIndirectIterator<false>(cvec.begin());
        static_assert(std::is_assignable<decltype(*b), int>::value, "");
    }
    {
        auto b = util::makeIndirectIterator<false>(vec.cbegin());
        static_assert(std::is_assignable<decltype(*b), int>::value, "");
    }
    {
        auto b = util::makeIndirectIterator<false>(cvec.cbegin());
        static_assert(std::is_assignable<decltype(*b), int>::value, "");
    }
}

}  // namespace inviwo
