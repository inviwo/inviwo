/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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
#include <inviwo/core/util/document.h>
#include <inviwo/core/util/typetraits.h>
#include <inviwo/core/util/isstreaminsertable.h>
#include <inviwo/core/util/transparentmaps.h>

#include <functional>
#include <sstream>
#include <type_traits>
#include <string>

namespace inviwo::utildoc {

namespace detail {

template <typename T>
std::string convert(T&& val)
    requires util::is_stream_insertable<T>::value
{
    std::stringstream value;
    value << std::boolalpha << std::forward<T>(val);
    return value.str();
}
template <typename T>
std::string convert(T&& /*val*/)
    requires(!util::is_stream_insertable<T>::value)
{
    return "???";
}

}  // namespace detail

class IVW_CORE_API TableBuilder {
public:
    struct IVW_CORE_API Wrapper {
        std::string data_;

    protected:
        template <typename T>
        explicit Wrapper(T&& data)
            requires(!std::is_same_v<T, Wrapper>)
            : data_(detail::convert(std::forward<T>(data))) {}
        explicit Wrapper(std::string data);
        explicit Wrapper(const char* data);
        explicit Wrapper(std::string_view data);
    };

    struct IVW_CORE_API ArrributeWrapper : Wrapper {
        template <typename T>
        ArrributeWrapper(const UnorderedStringMap<std::string>& attributes, T&& data)
            : Wrapper(std::forward<T>(data)), attributes_(attributes) {}
        UnorderedStringMap<std::string> attributes_;
    };
    struct IVW_CORE_API Header : Wrapper {
        template <typename T>
        explicit Header(T&& data)
            requires(!std::is_same_v<T, Header>)
            : Wrapper(std::forward<T>(data)) {}
    };

    struct Span_t {};

    TableBuilder(Document::DocumentHandle handle, Document::PathComponent pos,
                 const UnorderedStringMap<std::string>& attributes = {});

    explicit TableBuilder(Document::DocumentHandle table);

    template <typename... Args>
    Document::DocumentHandle operator()(Document::PathComponent pos, Args&&... args) {
        auto row = table_.insert(std::move(pos), "tr");
        tablerow(row, std::forward<Args>(args)...);
        return row;
    }

    template <typename... Args>
    Document::DocumentHandle operator()(Args&&... args) {
        return operator()(Document::PathComponent::end(), std::forward<Args>(args)...);
    }

private:
    template <typename T, typename... Args>
    void tablerow(Document::DocumentHandle& w, T&& val, Args&&... args) const {
        tabledata(w, std::forward<T>(val));
        tablerow(w, std::forward<Args>(args)...);
    }

    template <typename T>
    void tablerow(Document::DocumentHandle& w, T&& val) const {
        tabledata(w, std::forward<T>(val));
    }

    static void tabledata(Document::DocumentHandle& row, const std::string& val);
    static void tabledata(Document::DocumentHandle& row, const char* val);

    template <typename T>
    static void tabledata(Document::DocumentHandle& row, T&& val)
        requires(!std::is_base_of_v<Wrapper, std::decay_t<T>>)
    {
        row.insert(Document::PathComponent::end(), "td", detail::convert(std::forward<T>(val)));
    }
    static void tabledata(Document::DocumentHandle& row, Span_t val);

    static void tabledata(Document::DocumentHandle& row, const ArrributeWrapper& val);
    static void tabledata(Document::DocumentHandle& row, const Header& val);

    Document::DocumentHandle table_;
};

}  // namespace inviwo::utildoc
