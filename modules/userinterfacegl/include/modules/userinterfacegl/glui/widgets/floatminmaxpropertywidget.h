/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2024 Inviwo Foundation
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

#include <inviwo/core/properties/minmaxproperty.h>             // for FloatMinMaxProperty
#include <inviwo/core/properties/propertyobserver.h>           // for PropertyObserver
#include <inviwo/core/properties/propertywidget.h>             // for PropertyWidget
#include <inviwo/core/util/glmvec.h>                           // for ivec2, vec2
#include <modules/userinterfacegl/glui/element.h>              // for UIOrientation, UIOrient...
#include <modules/userinterfacegl/glui/widgets/rangeslider.h>  // for RangeSlider

#include <string>  // for string

namespace inviwo {
class Processor;
class Property;

namespace glui {
class Renderer;

/**
 * \class FloatMinMaxPropertyWidget
 * \brief glui property widget for a float minmax property using glui::RangeSlider
 */
class IVW_MODULE_USERINTERFACEGL_API FloatMinMaxPropertyWidget : public RangeSlider,
                                                                 public PropertyWidget,
                                                                 public PropertyObserver {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.glui.FloatMinMaxPropertyWidget"};

    FloatMinMaxPropertyWidget(FloatMinMaxProperty& property, Processor& processor,
                              Renderer& uiRenderer, const ivec2& extent = ivec2(100, 24),
                              UIOrientation orientation = UIOrientation::Horizontal);
    virtual ~FloatMinMaxPropertyWidget() = default;

    virtual void updateFromProperty() override;

    // PropertyObservable overrides
    virtual void onSetVisible(Property* property, bool visible) override;
    virtual void onSetDisplayName(Property* property, const std::string& displayName) override;
    virtual void onSetReadOnly(Property* property, bool readonly) override;

private:
    float sliderToRepr(int val) const;
    int reprToSlider(float val) const;

    vec2 sliderToRepr(const ivec2& val) const;
    ivec2 reprToSlider(const vec2& val) const;

    int reprSeparationToSlider() const;

    const int sliderMax_;

    FloatMinMaxProperty* property_;
};

}  // namespace glui

}  // namespace inviwo
