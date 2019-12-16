/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <warn/pop>

#include <ostream>
#include <string>

namespace inviwo {

namespace detail {

template <typename PairType>
class ZipPairMatcherImpl : public ::testing::MatcherInterface<PairType> {

public:
    typedef GTEST_REMOVE_REFERENCE_AND_CONST_(PairType) RawPairType;
    typedef typename RawPairType::first_type FirstType;
    typedef typename RawPairType::second_type SecondType;

    template <typename FirstMatcher, typename SecondMatcher>
    ZipPairMatcherImpl(FirstMatcher first_matcher, SecondMatcher second_matcher)
        : first_matcher_(::testing::SafeMatcherCast<const FirstType&>(first_matcher))
        , second_matcher_(::testing::SafeMatcherCast<const SecondType&>(second_matcher)) {}

    bool MatchAndExplain(PairType a_pair, ::testing::MatchResultListener* listener) const override {
        if (!listener->IsInterested()) {
            return first_matcher_.Matches(a_pair.first()) &&
                   second_matcher_.Matches(a_pair.second());
        }

        ::testing::StringMatchResultListener first_inner_listener;
        if (!first_matcher_.MatchAndExplain(a_pair.first(), &first_inner_listener)) {
            *listener << "whose first field does not match";
            ::testing::internal::PrintIfNotEmpty(first_inner_listener.str(), listener->stream());
            return false;
        }
        ::testing::StringMatchResultListener second_inner_listener;
        if (!second_matcher_.MatchAndExplain(a_pair.second(), &second_inner_listener)) {
            *listener << "whose second field does not match";
            ::testing::internal::PrintIfNotEmpty(second_inner_listener.str(), listener->stream());
            return false;
        }
        ExplainSuccess(first_inner_listener.str(), second_inner_listener.str(), listener);
        return true;
    }

    // Describes what this matcher does.
    void DescribeTo(::std::ostream* os) const override {
        *os << "has a first field that ";
        first_matcher_.DescribeTo(os);
        *os << ", and has a second field that ";
        second_matcher_.DescribeTo(os);
    }

    // Describes what the negation of this matcher does.
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "has a first field that ";
        first_matcher_.DescribeNegationTo(os);
        *os << ", or has a second field that ";
        second_matcher_.DescribeNegationTo(os);
    }

private:
    void ExplainSuccess(const std::string& first_explanation, const std::string& second_explanation,
                        ::testing::MatchResultListener* listener) const {
        *listener << "whose both fields match";
        if (first_explanation != "") {
            *listener << ", where the first field is a value " << first_explanation;
        }
        if (second_explanation != "") {
            *listener << ", ";
            if (first_explanation != "") {
                *listener << "and ";
            } else {
                *listener << "where ";
            }
            *listener << "the second field is a value " << second_explanation;
        }
    }

    const ::testing::Matcher<const FirstType&> first_matcher_;
    const ::testing::Matcher<const SecondType&> second_matcher_;

    GTEST_DISALLOW_ASSIGN_(ZipPairMatcherImpl);
};

template <typename FirstMatcher, typename SecondMatcher>
class ZipPairMatcher {
public:
    ZipPairMatcher(FirstMatcher first_matcher, SecondMatcher second_matcher)
        : first_matcher_(first_matcher), second_matcher_(second_matcher) {}

    template <typename PairType>
    operator ::testing::Matcher<PairType>() const {
        return ::testing::Matcher<PairType>(
            new ZipPairMatcherImpl<const PairType&>(first_matcher_, second_matcher_));
    }

private:
    const FirstMatcher first_matcher_;
    const SecondMatcher second_matcher_;

    GTEST_DISALLOW_ASSIGN_(ZipPairMatcher);
};

}  // namespace detail

template <typename FirstMatcher, typename SecondMatcher>
inline detail::ZipPairMatcher<FirstMatcher, SecondMatcher> ZipPair(FirstMatcher first_matcher,
                                                                   SecondMatcher second_matcher) {
    return detail::ZipPairMatcher<FirstMatcher, SecondMatcher>(first_matcher, second_matcher);
}

}  // namespace inviwo