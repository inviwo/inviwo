/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>

#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>

#include <modules/basegl/datastructures/splittersettings.h>

namespace inviwo {

/**
 * \ingroup properties
 * \brief composite property holding all settings of a splitter handle
 */
class IVW_MODULE_BASEGL_API SplitterProperty : public BoolCompositeProperty,
                                               public SplitterSettings {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    SplitterProperty(const std::string& identifier, const std::string& displayName,
                     bool checked = true, splitter::Style style = splitter::Style::Divider,
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

    TemplateOptionProperty<splitter::Style> style_;
    FloatVec4Property color_;
    FloatVec4Property bgColor_;
    FloatVec4Property hoverColor_;
    FloatProperty length_;
    FloatProperty width_;
    FloatProperty triSize_;
};

}  // namespace inviwo
