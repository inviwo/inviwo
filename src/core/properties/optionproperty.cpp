/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#include <inviwo/core/properties/optionproperty.h>

#include <fmt/core.h>

namespace inviwo {

BaseOptionProperty::BaseOptionProperty(std::string_view identifier, std::string_view displayName,
                                       Document help, InvalidationLevel invalidationLevel,
                                       PropertySemantics semantics)
    : Property(identifier, displayName, std::move(help), invalidationLevel, semantics) {}

BaseOptionProperty::BaseOptionProperty(const BaseOptionProperty& rhs) = default;

BaseOptionProperty::~BaseOptionProperty() = default;

void BaseOptionProperty::set(const BaseOptionProperty* srcProperty) {
    size_t option = std::min(srcProperty->getSelectedIndex(), size() - 1);
    if (option != getSelectedIndex()) {
        setSelectedIndex(option);
        propertyModified();
    }
}

void BaseOptionProperty::set(const Property* srcProperty) {
    if (auto optionSrcProp = dynamic_cast<const BaseOptionProperty*>(srcProperty)) {
        set(optionSrcProp);
    }
}

bool BaseOptionProperty::empty() const { return size() == 0; }

/// @cond
template class IVW_CORE_TMPL_INST OptionPropertyOption<unsigned int>;
template class IVW_CORE_TMPL_INST OptionPropertyOption<int>;
template class IVW_CORE_TMPL_INST OptionPropertyOption<size_t>;
template class IVW_CORE_TMPL_INST OptionPropertyOption<float>;
template class IVW_CORE_TMPL_INST OptionPropertyOption<double>;
template class IVW_CORE_TMPL_INST OptionPropertyOption<std::string>;

template class IVW_CORE_TMPL_INST OptionProperty<unsigned int>;
template class IVW_CORE_TMPL_INST OptionProperty<int>;
template class IVW_CORE_TMPL_INST OptionProperty<size_t>;
template class IVW_CORE_TMPL_INST OptionProperty<float>;
template class IVW_CORE_TMPL_INST OptionProperty<double>;
template class IVW_CORE_TMPL_INST OptionProperty<std::string>;
/// @endcond

std::vector<OptionPropertyIntOption> util::enumeratedOptions(std::string_view name, size_t count,
                                                             int start, int step) {
    std::vector<OptionPropertyIntOption> res;
    for (size_t i = 0; i < count; ++i) {
        res.emplace_back(fmt::format("{}{}", toLower(name), i + 1),
                         fmt::format("{} {}", name, i + 1), start + static_cast<int>(i) * step);
    }
    return res;
}

}  // namespace inviwo
