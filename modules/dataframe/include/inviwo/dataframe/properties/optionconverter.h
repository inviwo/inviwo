/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2022 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 *FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#pragma once

#include <inviwo/core/properties/propertyconverter.h>          // for TemplatePropertyConverter
#include <inviwo/dataframe/properties/columnoptionproperty.h>  // for ColumnOptionProperty

#include <cstddef>                                             // for size_t

namespace inviwo {

template <typename OptionProperty>
class ColumnOptionToOptionConverter
    : public TemplatePropertyConverter<ColumnOptionProperty, OptionProperty> {
protected:
protected:
    virtual void convertimpl(const ColumnOptionProperty* src, OptionProperty* dst) const override {
        dst->setSelectedIndex(glm::clamp<size_t>(0, dst->size() - 1, src->getSelectedIndex()));
    }
};

template <typename OptionProperty>
class OptionToColumnOptionConverter
    : public TemplatePropertyConverter<OptionProperty, ColumnOptionProperty> {
protected:
protected:
    virtual void convertimpl(const OptionProperty* src, ColumnOptionProperty* dst) const override {
        dst->setSelectedIndex(glm::clamp<size_t>(0, dst->size() - 1, src->getSelectedIndex()));
    }
};

}  // namespace inviwo
