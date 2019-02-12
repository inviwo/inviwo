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

#include <inviwo/core/util/zip.h>

#include <array>
#include <vector>
#include <list>
#include <forward_list>

#include <numeric>
#include <tuple>
#include <memory>
#include <utility>

template <size_t n, typename... T>
typename std::enable_if<(n >= sizeof...(T))>::type print_tuple(std::ostream&,
                                                               const std::tuple<T...>&) {}

template <size_t n, typename... T>
typename std::enable_if<(n > 0 && n < sizeof...(T))>::type print_tuple(
    std::ostream& os, const std::tuple<T...>& tup) {
    os << ", " << std::get<n>(tup);
    print_tuple<n + 1>(os, tup);
}

template <size_t n, typename... T>
typename std::enable_if<(n == 0)>::type print_tuple(std::ostream& os, const std::tuple<T...>& tup) {
    os << std::get<n>(tup);
    print_tuple<n + 1>(os, tup);
}

template <typename... T>
std::ostream& operator<<(std::ostream& os, const std::tuple<T...>& tup) {
    os << "[";
    print_tuple<0>(os, tup);
    return os << "]";
}

template <typename... T>
std::ostream& operator<<(std::ostream& os, const inviwo::util::detailzip::proxy<T...>& p) {
    os << "[";
    print_tuple<0>(os, p.data);
    return os << "]";
}

// no copy/move
struct S {
    S() = delete;
    S(const S&) = delete;
    S(S&&) = delete;
    S& operator=(const S&) = delete;
    S& operator=(S&&) = delete;

    friend constexpr bool operator==(const S& a, const S& b) noexcept { return a.a_ == b.a_; }

    constexpr int get() const noexcept { return a_; }

    constexpr explicit S(int a) noexcept : a_{a} {}
    int a_ = 0;
};
::std::ostream& operator<<(::std::ostream& os, const S& s) { return os << "S(" << s.a_ << ")"; }

// default type
struct D {
    constexpr D() noexcept = default;
    constexpr D(const D&) noexcept = default;
    constexpr D(D&&) noexcept = default;
    D& operator=(const D&) noexcept = default;
    D& operator=(D&&) noexcept = default;

    friend constexpr bool operator==(const D& a, const D& b) noexcept { return a.a_ == b.a_; }

    constexpr int get() const noexcept { return a_; }

    constexpr explicit D(int a) noexcept : a_{a} {}
    int a_ = 0;
};
::std::ostream& operator<<(::std::ostream& os, const D& d) { return os << "D(" << d.a_ << ")"; }

constexpr const std::array<int, 5> ints{{1, 2, 3, 4, 5}};
constexpr const std::array<D, 5> ds{{D{10}, D{20}, D{30}, D{40}, D{50}}};

namespace inviwo {

template <typename Zipped>
void forwadTest(Zipped&& iter) {
    using Iter = typename std::decay_t<decltype(iter)>::iterator;
    using util::detailzip::proxy;

    // Iterator default constructible
    Iter defaultConstruct{};

    // Iterator requirements
    Iter copy{defaultConstruct};
    Iter assign = defaultConstruct;
    std::swap(copy, assign);

    {
        auto i = iter.begin();
        auto j = iter.end();
        EXPECT_FALSE(i == j);
        for (size_t c = 0; c < ints.size(); ++c) {
            auto ref = proxy<int, D>{ints[c], ds[c]};
            EXPECT_EQ(ref, *i);
            Iter& i2 = ++i;
            EXPECT_EQ(i2, i);
        }
    }
    {
        auto i = iter.begin();
        auto j = iter.end();
        EXPECT_FALSE(i == j);
        // InputIterator requirements
        EXPECT_EQ(true, i != j);
        EXPECT_EQ(false, i == j);
        typename Iter::value_type vt1 = *i;
        typename Iter::value_type vt2 = *i++;
        auto ref = proxy<int, D>{1, D{10}};
        EXPECT_EQ(ref, vt1);
        EXPECT_EQ(ref, vt2);

        i = iter.begin();
        for (size_t c = 0; c < ints.size(); ++c) {
            i++;
        }
        EXPECT_EQ(true, i == j);
    }
    // ForwardIterator requirements
    // same as InputIterator + multi pass
    {
        auto i = iter.begin();
        auto j = iter.end();
        EXPECT_FALSE(i == j);
        for (size_t c = 0; c < ints.size(); ++c) {
            auto ref = proxy<int, D>{ints[c], ds[c]};
            EXPECT_EQ(ref, *i);
            Iter i1 = i;
            Iter i2 = i++;
            EXPECT_EQ(i2, i1);
        }
    }
}

template <typename Zipped>
void bidirectionalTest(Zipped&& iter) {
    using util::detailzip::proxy;
    {
        auto i = iter.begin();
        auto j = iter.end();
        EXPECT_FALSE(i == j);
        std::advance(i, 2);

        auto ref0 = proxy<int, D>{3, D{30}};
        EXPECT_EQ(ref0, *i);

        using Iter = typename std::decay_t<decltype(iter)>::iterator;
        // BidirectionalIterator requirements
        Iter& i4 = --i;

        auto ref1 = proxy<int, D>{2, D{20}};
        EXPECT_EQ(ref1, *i4);
        EXPECT_EQ(ref1, *i);

        typename Iter::value_type vt1 = *i;
        typename Iter::value_type vt2 = *i--;

        EXPECT_EQ(ref1, vt1);
        EXPECT_EQ(ref1, vt2);

        auto ref2 = proxy<int, D>{1, D{10}};
        EXPECT_EQ(ref2, *i);
    }
    {
        auto i = iter.begin();
        auto j1 = iter.end();
        auto j2 = iter.end();
        j2--;
        --j1;

        for (int c = static_cast<int>(ints.size()) - 1; c >= 1; --c) {
            auto ref3 = proxy<int, D>{ints[c], ds[c]};
            EXPECT_EQ(ref3, *j1);
            EXPECT_EQ(ref3, *j2);
            EXPECT_EQ(true, j1 == j2);

            auto val = *j2--;
            EXPECT_EQ(true, val == *j1);
            --j1;

            EXPECT_EQ(true, j1 == j2);
        }
        EXPECT_EQ(*i, *j1);
        EXPECT_EQ(true, i == j1);
        EXPECT_EQ(true, i == j2);
    }
}

template <typename Zipped>
void randomAccessTest(Zipped&& iter) {
    using util::detailzip::proxy;
    using Iter = typename std::decay_t<decltype(iter)>::iterator;
    // RandomAccessIterator requirements
    using DT = typename Iter::difference_type;

    for (DT n = 0; n < static_cast<DT>(ints.size()); ++n) {

        {
            auto i = iter.begin();
            Iter& i1 = i += n;
            auto ref = proxy<int, D>{ints[n], ds[n]};
            EXPECT_EQ(ref, *i1);
        }

        {
            auto i = iter.begin();
            Iter i2 = i + n;
            Iter i3 = n + i;
            auto ref = proxy<int, D>{ints[n], ds[n]};
            EXPECT_EQ(ref, *i2);
            EXPECT_EQ(ref, *i3);
        }
        {
            auto i = iter.end();
            Iter i5 = i - (n + 1);
            auto ref = proxy<int, D>{ints[ints.size() - n - 1], ds[ints.size() - n - 1]};
            EXPECT_EQ(ref, *i5);
        }
        {
            auto i = iter.end();
            Iter& i4 = i -= (n + 1);
            auto ref = proxy<int, D>{ints[ints.size() - n - 1], ds[ints.size() - n - 1]};
            EXPECT_EQ(ref, *i4);
        }
    }
    {
        auto j = iter.end();
        auto i = iter.begin();
        for (DT n = 0; n < static_cast<DT>(ints.size()); ++n) {
            DT size = j - i;
            EXPECT_EQ(5 - n, size);
            ++i;
        }
    }

    {
        auto i = iter.begin();
        for (DT n = 0; n < static_cast<DT>(ints.size()); ++n) {
            typename Iter::reference vr1 = i[n];
            auto ref = proxy<int, D>{ints[n], ds[n]};
            EXPECT_EQ(ref, vr1);
            EXPECT_EQ(ref, i[n]);
        }
    }
    {
        auto i = iter.begin();
        auto j = iter.end();
        EXPECT_TRUE(i < j);
        EXPECT_FALSE(i > j);
        EXPECT_TRUE(i <= j);
        EXPECT_FALSE(i >= j);
    }
}

TEST(ZipIterTest, ForwardIter) {
    std::forward_list<int> fl1{ints.begin(), ints.end()};
    std::forward_list<D> fl2{ds.begin(), ds.end()};

    auto iter = util::zip(fl1, fl2);
    forwadTest(iter);
}

TEST(ZipIterTest, BidirectionalIter) {
    std::list<int> bl1{ints.begin(), ints.end()};
    std::list<D> bl2{ds.begin(), ds.end()};

    auto iter = util::zip(bl1, bl2);
    forwadTest(iter);
    bidirectionalTest(iter);
}

TEST(ZipIterTest, RandomAccessIter) {
    std::vector<int> bl1{ints.begin(), ints.end()};
    std::vector<D> bl2{ds.begin(), ds.end()};

    auto iter = util::zip(bl1, bl2);
    forwadTest(iter);
    bidirectionalTest(iter);
    randomAccessTest(iter);
}

TEST(ZipIterTest, Minimal) {
    std::vector<int> a(10);
    std::iota(a.begin(), a.end(), 0);
    int count = 0;
    for (auto&& i : util::zip(a)) {
        EXPECT_EQ(count, get<0>(i));
        ++count;
    }
}

TEST(ZipIterTest, Pair) {
    std::vector<int> a(10);
    std::vector<int> b(10);

    std::iota(a.begin(), a.end(), 0);
    std::iota(b.begin(), b.end(), 10);

    int count = 0;
    for (auto&& i : util::zip(a, b)) {
        EXPECT_EQ(count, get<0>(i));
        EXPECT_EQ(count + 10, get<1>(i));
        ++count;
    }
}

TEST(ZipIterTest, NoCopy) {
    std::vector<std::unique_ptr<S>> a;
    std::vector<int> b;

    for (int i = 0; i < 10; ++i) {
        a.push_back(std::make_unique<S>(i));
        b.push_back(i + 10);
    }

    int count = 0;
    for (auto&& i : util::zip(a, b)) {
        EXPECT_EQ(count, get<0>(i)->a_);
        EXPECT_EQ(count + 10, get<1>(i));
        ++count;
    }
}

TEST(ZipIterTest, Sort) {
    std::vector<int> a(10);
    std::vector<int> b(10);

    std::iota(a.begin(), a.end(), 0);
    std::iota(b.begin(), b.end(), 10);

    std::reverse(a.begin(), a.end());

    auto z = util::zip(a, b);
    std::sort(z.begin(), z.end(), [](auto&& a, auto&& b) -> bool { return get<0>(a) < get<0>(b); });

    EXPECT_EQ(19, b.front());
    EXPECT_EQ(10, b.back());
}

TEST(SequencerTest, Ranges) {
    int count = 0;
    auto r1 = util::make_sequence(0, 10, 1);
    auto r2 = util::make_sequence(10, 20, 1);
    for (auto&& i : util::zip(r1, r2)) {
        EXPECT_EQ(count, get<0>(i));
        EXPECT_EQ(count + 10, get<1>(i));
        ++count;
    }
}

TEST(SequencerTest, DifferenLenght1) {
    int count = 0;
    auto r1 = util::make_sequence(0, 10, 1);
    auto r2 = util::make_sequence(10, 30, 1);
    for (auto&& i : util::zip(r1, r2)) {
        EXPECT_EQ(count, get<0>(i));
        EXPECT_EQ(count + 10, get<1>(i));
        ++count;
    }
}
TEST(SequencerTest, DifferenLenght2) {
    int count = 0;
    auto r1 = util::make_sequence(0, 20, 1);
    auto r2 = util::make_sequence(10, 20, 1);
    for (auto&& i : util::zip(r1, r2)) {
        EXPECT_EQ(count, get<0>(i));
        EXPECT_EQ(count + 10, get<1>(i));
        ++count;
    }
}

TEST(SequencerTest, Temporaries) {
    int count = 0;
    for (auto&& i : util::zip(util::make_sequence(0, 20, 1), util::make_sequence(10, 20, 1))) {
        EXPECT_EQ(count, get<0>(i));
        EXPECT_EQ(count + 10, get<1>(i));
        ++count;
    }
}

TEST(SequencerTest, increments1) {
    int count = 0;
    for (auto&& i : util::make_sequence(0, 20, 2)) {
        EXPECT_EQ(count, i);
        count += 2;
    }
}

TEST(SequencerTest, increments2) {
    int count = 0;
    for (auto&& i : util::make_sequence(0, 19, 2)) {
        EXPECT_EQ(count, i);
        count += 2;
    }
}

TEST(SequencerTest, increments3) {
    int count = 0;
    for (auto&& i : util::make_sequence(0, -19, -2)) {
        EXPECT_EQ(count, i);
        count -= 2;
    }
}
}  // namespace inviwo
