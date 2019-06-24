/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/userinterfacegl/userinterfaceglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/userinterfacegl/glui/widgets/rangeslider.h>

#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/propertywidget.h>
#include <inviwo/core/properties/propertyobserver.h>

namespace inviwo {

namespace glui {

/**
 * \brief glui property widget for a double minmax property using glui::RangeSlider
 */
class IVW_MODULE_USERINTERFACEGL_API DoubleMinMaxPropertyWidget : public RangeSlider,
                                                                  public PropertyWidget,
                                                                  public PropertyObserver {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    DoubleMinMaxPropertyWidget(DoubleMinMaxProperty &property, Processor &processor,
                               Renderer &uiRenderer, const ivec2 &extent = ivec2(100, 24),
                               UIOrientation orientation = UIOrientation::Horizontal);
    virtual ~DoubleMinMaxPropertyWidget() = default;

    virtual void updateFromProperty() override;

    // PropertyObservable overrides
    virtual void onSetVisible(Property *property, bool visible) override;
    virtual void onSetDisplayName(Property *property, const std::string &displayName) override;
    virtual void onSetReadOnly(Property *property, bool readonly) override;

private:
    double sliderToRepr(int val) const;
    int reprToSlider(double val) const;

    dvec2 sliderToRepr(const ivec2 &val) const;
    ivec2 reprToSlider(const dvec2 &val) const;

    int reprSeparationToSlider() const;

    const int sliderMax_;

    DoubleMinMaxProperty *property_;
};

}  // namespace glui

}  // namespace inviwo
