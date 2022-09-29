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

#include <modules/userinterfacegl/userinterfaceglmoduledefine.h>  // for IVW_MODULE_USERINTERFAC...

#include <inviwo/core/properties/propertyobserver.h>              // for PropertyObserver
#include <inviwo/core/properties/propertywidget.h>                // for PropertyWidget
#include <inviwo/core/util/glmvec.h>                              // for ivec2
#include <modules/userinterfacegl/glui/widgets/checkbox.h>        // for CheckBox

#include <string>                                                 // for string

namespace inviwo {
class BoolProperty;
class Processor;
class Property;

namespace glui {
class Renderer;

/**
 * \class BoolPropertyWidget
 * \brief glui property widget for a bool property using glui::CheckBox
 */
class IVW_MODULE_USERINTERFACEGL_API BoolPropertyWidget : public CheckBox,
                                                          public PropertyWidget,
                                                          public PropertyObserver {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    BoolPropertyWidget(BoolProperty& property, Processor& processor, Renderer& uiRenderer,
                       const ivec2& extent = ivec2(24, 24));
    virtual ~BoolPropertyWidget() = default;

    virtual void updateFromProperty() override;

    // PropertyObservable overrides
    virtual void onSetVisible(Property* property, bool visible) override;
    virtual void onSetDisplayName(Property* property, const std::string& displayName) override;
    virtual void onSetReadOnly(Property* property, bool readonly) override;

private:
    BoolProperty* property_;
};

}  // namespace glui

}  // namespace inviwo
