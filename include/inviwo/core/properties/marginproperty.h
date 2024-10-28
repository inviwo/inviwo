/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>

#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/util/glmvec.h>

#include <string>
#include <string_view>
#include <utility>

namespace inviwo {

/**
 * \ingroup properties
 * \brief Property for keeping track of margins
 *
 * Set functions use the same ordering as is common in CSS, that is clockwise direction starting
 * from top (top, right, bottom, left).
 */
class IVW_CORE_API MarginProperty : public CompositeProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static const std::string classIdentifier;

    MarginProperty(std::string_view identifier, std::string_view displayName, float top = 20.0f,
                   float right = 20.0f, float bottom = 20.0f, float left = 20.0f,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                   PropertySemantics semantics = PropertySemantics::Default);

    MarginProperty(const MarginProperty& rhs);
    virtual MarginProperty* clone() const override;

    virtual ~MarginProperty() = default;
    /*
     * Set margins and adjust min/max values if necessary
     */
    void setMargins(float top, float right, float bottom, float left);

    void setTop(float top);
    void setRight(float right);
    void setBottom(float bottom);
    void setLeft(float left);

    float getTop() const;
    float getRight() const;
    float getBottom() const;
    float getLeft() const;

    vec2 getLowerLeftMargin() const;
    vec2 getUpperRightMargin() const;

    void setLowerLeftMargin(vec2 lowerLeft);
    void setUpperRightMargin(vec2 upperRight);

    vec4 getAsVec4() const;

    /**
     * Returns the lower left point and the upper right point;
     */
    std::pair<vec2, vec2> getRect(vec2 size) const;

    /**
     * Returns the given size with the margins subtracted.
     */
    vec2 getSize(vec2 size) const;

    FloatProperty top_;
    FloatProperty right_;
    FloatProperty bottom_;
    FloatProperty left_;
};

}  // namespace inviwo
