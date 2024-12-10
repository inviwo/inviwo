/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2024 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEGL_API

#include <inviwo/core/properties/boolcompositeproperty.h>    // for BoolCompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>        // for InvalidationLevel, Invalidat...
#include <inviwo/core/properties/optionproperty.h>           // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>          // for FloatProperty, FloatVec4Prop...
#include <inviwo/core/properties/propertysemantics.h>        // for PropertySemantics, PropertyS...
#include <inviwo/core/util/glmvec.h>                         // for vec4
#include <inviwo/core/util/staticstring.h>                   // for operator+
#include <modules/basegl/datastructures/splittersettings.h>  // for Style, Style::Divider, Split...

#include <functional>   // for __base
#include <string>       // for operator==, string
#include <string_view>  // for operator==, string_view
#include <vector>       // for operator!=, vector, operator==

namespace inviwo {

/**
 * \ingroup properties
 * \brief composite property holding all settings of a splitter handle
 */
class IVW_MODULE_BASEGL_API SplitterProperty : public BoolCompositeProperty,
                                               public SplitterSettings {
public:
    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.SplitterProperty"};

    SplitterProperty(std::string_view identifier, std::string_view displayName, bool checked = true,
                     splitter::Style style = splitter::Style::Divider,
                     vec4 color = vec4(0.71f, 0.81f, 0.85f, 1.0f),
                     vec4 bgColor = vec4(0.27f, 0.3f, 0.31f, 1.0f),
                     InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                     PropertySemantics semantics = PropertySemantics::Default);
    SplitterProperty(const SplitterProperty& rhs);
    virtual SplitterProperty* clone() const override;
    virtual ~SplitterProperty() = default;

    virtual bool enabled() const override;
    virtual splitter::Style getStyle() const override;
    virtual vec4 getColor() const override;
    virtual vec4 getBackgroundColor() const override;
    virtual vec4 getHoverColor() const override;
    virtual float getLength() const override;
    virtual float getWidth() const override;
    virtual float getTriangleSize() const override;

    OptionProperty<splitter::Style> style_;
    FloatVec4Property color_;
    FloatVec4Property bgColor_;
    FloatVec4Property hoverColor_;
    FloatProperty length_;
    FloatProperty width_;
    FloatProperty triSize_;
};

}  // namespace inviwo
