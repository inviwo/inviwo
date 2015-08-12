/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

PropertyClassIdentifier(BoolCompositeProperty, "org.inviwo.BoolCompositeProperty");

BoolCompositeProperty::BoolCompositeProperty(std::string identifier, std::string displayName,
                                     bool checked,
                                     InvalidationLevel invalidationLevel,
                                     PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , checked_("checked", "checked", checked, invalidationLevel, semantics) 
{
    checked_.setVisible(false);
    this->addProperty(checked_);
    checked_.onChange([this]() {
        Property::propertyModified();
    });
}

BoolCompositeProperty* BoolCompositeProperty::clone() const {
    return new BoolCompositeProperty(*this);
}

BoolCompositeProperty::~BoolCompositeProperty() {}

std::string BoolCompositeProperty::getClassIdentifierForWidget() const{
    return getClassIdentifier();
}

bool BoolCompositeProperty::isChecked() const {
    return checked_.get();
}

void BoolCompositeProperty::setChecked(bool checked) {
    if (checked_.get() != checked) {
        checked_.set(checked);
    }
}

BoolProperty* BoolCompositeProperty::getBoolProperty() { return &checked_; }

} // namespace
