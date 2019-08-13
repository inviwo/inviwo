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

#include <inviwo/core/properties/optionproperty.h>

namespace inviwo {

BaseOptionProperty::BaseOptionProperty(const std::string& identifier,
                                       const std::string& displayName,
                                       InvalidationLevel invalidationLevel,
                                       PropertySemantics semantics)
    : Property(identifier, displayName, invalidationLevel, semantics) {}

BaseOptionProperty::BaseOptionProperty(const BaseOptionProperty& rhs) = default;

BaseOptionProperty::~BaseOptionProperty() = default;

void BaseOptionProperty::set(const Property* srcProperty) {
    if (auto optionSrcProp = dynamic_cast<const BaseOptionProperty*>(srcProperty)) {
        size_t option = std::min(optionSrcProp->getSelectedIndex(), size() - 1);
        if (option != getSelectedIndex()) {
            setSelectedIndex(option);
            propertyModified();
        }
    }
}

template class IVW_CORE_TMPL_INST OptionPropertyOption<unsigned int>;
template class IVW_CORE_TMPL_INST OptionPropertyOption<int>;
template class IVW_CORE_TMPL_INST OptionPropertyOption<size_t>;
template class IVW_CORE_TMPL_INST OptionPropertyOption<float>;
template class IVW_CORE_TMPL_INST OptionPropertyOption<double>;
template class IVW_CORE_TMPL_INST OptionPropertyOption<std::string>;

template class IVW_CORE_TMPL_INST TemplateOptionProperty<unsigned int>;
template class IVW_CORE_TMPL_INST TemplateOptionProperty<int>;
template class IVW_CORE_TMPL_INST TemplateOptionProperty<size_t>;
template class IVW_CORE_TMPL_INST TemplateOptionProperty<float>;
template class IVW_CORE_TMPL_INST TemplateOptionProperty<double>;
template class IVW_CORE_TMPL_INST TemplateOptionProperty<std::string>;

}  // namespace inviwo
