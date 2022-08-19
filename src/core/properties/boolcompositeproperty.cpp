/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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

#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/templateproperty.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

const std::string BoolCompositeProperty::classIdentifier = "org.inviwo.BoolCompositeProperty";
std::string BoolCompositeProperty::getClassIdentifier() const { return classIdentifier; }

BoolCompositeProperty::BoolCompositeProperty(std::string_view identifier,
                                             std::string_view displayName, Document help,
                                             bool checked, InvalidationLevel invalidationLevel,
                                             PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, std::move(help), invalidationLevel, semantics)
    , checked_("checked", "", checked, invalidationLevel, semantics) {
    addProperty(checked_);
}

BoolCompositeProperty::BoolCompositeProperty(std::string_view identifier,
                                             std::string_view displayName, bool checked,
                                             InvalidationLevel invalidationLevel,
                                             PropertySemantics semantics)
    : BoolCompositeProperty(identifier, displayName, Document{}, checked, invalidationLevel,
                            semantics) {}

BoolCompositeProperty::BoolCompositeProperty(const BoolCompositeProperty& rhs)
    : CompositeProperty(rhs), checked_{rhs.checked_} {
    addProperty(checked_);
}

BoolCompositeProperty* BoolCompositeProperty::clone() const {
    return new BoolCompositeProperty(*this);
}

BoolCompositeProperty::~BoolCompositeProperty() = default;

std::string BoolCompositeProperty::getClassIdentifierForWidget() const {
    return BoolCompositeProperty::classIdentifier;
}

bool BoolCompositeProperty::isChecked() const { return checked_.get(); }

void BoolCompositeProperty::setChecked(bool checked) { checked_.set(checked); }

BoolCompositeProperty::operator bool() const { return checked_.get(); }

BoolProperty* BoolCompositeProperty::getBoolProperty() { return &checked_; }

}  // namespace inviwo
