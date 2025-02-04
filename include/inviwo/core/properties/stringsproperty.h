/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2025 Inviwo Foundation
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
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/datastructures/unitsystem.h>
#include <inviwo/core/util/docutils.h>

#include <array>
#include <span>
#include <fmt/format.h>
#include <fmt/compile.h>

namespace inviwo {

namespace util {

template <size_t N>
std::span<const std::string_view, N> defaultValues() {
    static constexpr std::string_view empty = "";
    static constexpr std::array<std::string_view, N> values =
        util::make_array<N>([&](auto) { return empty; });
    return values;
}
}  // namespace util

/**
 * \brief CompositeProperty for a fixed set of StringProperties
 */
template <size_t N>
class StringsProperty : public CompositeProperty {
public:
    StringsProperty(std::string_view identifier, std::string_view displayName, Document help,
                    std::span<const std::string_view, N> values = util::defaultValues<N>(),
                    InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                    PropertySemantics semantics = PropertySemantics::Default);

    StringsProperty(std::string_view identifier, std::string_view displayName,
                    std::span<const std::string_view, N> values = util::defaultValues<N>(),
                    InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                    PropertySemantics semantics = PropertySemantics::Default);

    StringsProperty(const StringsProperty& rhs);
    virtual StringsProperty* clone() const override;
    virtual ~StringsProperty() = default;

    virtual Document getDescription() const override;
    virtual std::string_view getClassIdentifier() const override;
    virtual std::string_view getClassIdentifierForWidget() const override;

    std::array<StringProperty, N> strings;
};

template <size_t N>
struct PropertyTraits<StringsProperty<N>> {
    static std::string_view classIdentifier() {
        static constexpr auto cid = []() {
            constexpr auto format = FMT_COMPILE("org.inviwo.StringsProperty{}");
            constexpr size_t size = fmt::formatted_size(format, N);
            StaticString<size> res;
            fmt::format_to(res.data(), format, N);
            return res;
        }();
        return cid;
    }
};

template <size_t N>
std::string_view StringsProperty<N>::getClassIdentifier() const {
    return PropertyTraits<StringsProperty<N>>::classIdentifier();
}

template <size_t N>
std::string_view StringsProperty<N>::getClassIdentifierForWidget() const {
    return PropertyTraits<StringsProperty<N>>::classIdentifier();
}

template <size_t N>
StringsProperty<N>::StringsProperty(std::string_view identifier, std::string_view displayName,
                                    Document help, std::span<const std::string_view, N> values,
                                    InvalidationLevel invalidationLevel,
                                    PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, std::move(help), invalidationLevel, semantics)
    , strings{util::make_array<N>([&](auto i) {
        return StringProperty{util::defaultAxesNames[i], util::defaultAxesNames[i], values[i]};
    })} {

    for (auto& prop : strings) {
        addProperty(prop);
    }
}

template <size_t N>
StringsProperty<N>::StringsProperty(std::string_view identifier, std::string_view displayName,
                                    std::span<const std::string_view, N> values,
                                    InvalidationLevel invalidationLevel,
                                    PropertySemantics semantics)
    : StringsProperty(identifier, displayName, Document(), values, invalidationLevel, semantics) {}

template <size_t N>
StringsProperty<N>::StringsProperty(const StringsProperty& rhs)
    : CompositeProperty(rhs), strings(rhs.strings) {

    for (auto& prop : strings) {
        addProperty(prop);
    }
}

template <size_t N>
StringsProperty<N>* StringsProperty<N>::clone() const {
    return new StringsProperty<N>(*this);
}

template <size_t N>
Document StringsProperty<N>::getDescription() const {

    using P = Document::PathComponent;
    Document doc = Property::getDescription();

    utildoc::TableBuilder tb(doc.handle(), P::end(), {{"String", "Value"}});
    for (const auto& prop : strings) {
        tb(prop.getIdentifier(), prop.get());
    }

    return doc;
};

}  // namespace inviwo
