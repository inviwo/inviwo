/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2023 Inviwo Foundation
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

#include <ostream>
#include <iterator>
#include <string>

namespace inviwo {

namespace util {

// Based on http://codereview.stackexchange.com/questions/13176/infix-iterator-code
// and http://en.cppreference.com/w/cpp/experimental/ostream_joiner

template <class DelimT, class charT = char, class traits = std::char_traits<charT>>
class ostream_joiner {
    std::basic_ostream<charT, traits>* os;
    std::basic_string<charT> delimiter;
    bool need_delimiter = false;

public:
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = void;

    using char_type = charT;
    using traits_type = traits;
    using ostream_type = std::basic_ostream<charT, traits>;

    ostream_joiner(ostream_type& s, const DelimT& d) : os(&s), delimiter(d) {}
    ostream_joiner(ostream_type& s, DelimT&& d) : os(&s), delimiter(std::move(d)) {}

    template <typename T>
    ostream_joiner<DelimT, charT, traits>& operator=(const T& item) {
        if (need_delimiter) *os << delimiter;
        auto pos = os->tellp();
        *os << item;
        need_delimiter = (os->tellp() != pos);  // don't add delimiter if there were no output
        return *this;
    }

    ostream_joiner<DelimT, charT, traits>& operator*() { return *this; }
    ostream_joiner<DelimT, charT, traits>& operator++() { return *this; }
    ostream_joiner<DelimT, charT, traits>& operator++(int) { return *this; }
};

template <class charT, class traits, class DelimT>
ostream_joiner<std::decay_t<DelimT>, charT, traits> make_ostream_joiner(
    std::basic_ostream<charT, traits>& os, DelimT&& delimiter) {
    return ostream_joiner<std::decay_t<DelimT>, charT, traits>(os, delimiter);
}

}  // namespace util

}  // namespace inviwo
