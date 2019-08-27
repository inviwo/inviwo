/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <inviwo/core/properties/tfpropertyconcept.h>

namespace inviwo {

namespace util {

// TransferFunctionProperty
template <>
TFPrimitiveSet* TFPropertyModel<TransferFunctionProperty*>::getTFInternal() const {
    return &data_->get();
}
template <>
bool TFPropertyModel<TransferFunctionProperty*>::hasTFInternal() const {
    return true;
}
template <>
TransferFunctionProperty* TFPropertyModel<TransferFunctionProperty*>::getTFPropertyInternal()
    const {
    return data_;
}

// IsoValueProperty
template <>
TFPrimitiveSet* TFPropertyModel<IsoValueProperty*>::getIsovaluesInternal() const {
    return &data_->get();
}
template <>
bool TFPropertyModel<IsoValueProperty*>::hasIsovaluesInternal() const {
    return true;
}
template <>
bool TFPropertyModel<IsoValueProperty*>::supportsMaskInternal() const {
    return false;
}
template <>
void TFPropertyModel<IsoValueProperty*>::setMaskInternal(double, double) {}
template <>
const dvec2 TFPropertyModel<IsoValueProperty*>::getMaskInternal() const {
    return {};
}
template <>
void TFPropertyModel<IsoValueProperty*>::clearMaskInternal() {}

// IsoTFProperty
template <>
TFPrimitiveSet* TFPropertyModel<IsoTFProperty*>::getTFInternal() const {
    return &data_->tf_.get();
}
template <>
TFPrimitiveSet* TFPropertyModel<IsoTFProperty*>::getIsovaluesInternal() const {
    return &data_->isovalues_.get();
}
template <>
bool TFPropertyModel<IsoTFProperty*>::hasTFInternal() const {
    return true;
}
template <>
bool TFPropertyModel<IsoTFProperty*>::hasIsovaluesInternal() const {
    return true;
}
template <>
TransferFunctionProperty* TFPropertyModel<IsoTFProperty*>::getTFPropertyInternal() const {
    return &data_->tf_;
}

template class IVW_CORE_TMPL_INST TFPropertyModel<TransferFunctionProperty*>;
template class IVW_CORE_TMPL_INST TFPropertyModel<IsoValueProperty*>;
template class IVW_CORE_TMPL_INST TFPropertyModel<IsoTFProperty*>;

}  // namespace util

}  // namespace inviwo
