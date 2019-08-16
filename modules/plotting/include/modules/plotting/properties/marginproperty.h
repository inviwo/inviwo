/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_MARGINPROPERTY_H
#define IVW_MARGINPROPERTY_H

#include <modules/plotting/plottingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>

namespace inviwo {

namespace plot {

/**
 * \class MarginProperty
 * \brief Property for keeping track of margins
 *
 * For set functions, uses the same ordering as is common in CSS, clockwise starting from top eg
 * top, right, bottom, left
 */
class IVW_MODULE_PLOTTING_API MarginProperty : public CompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    MarginProperty(std::string identifier, std::string displayName, float top = 20.0f,
                   float right = 20.0f, float bottom = 20.0f, float left = 20.0f,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                   PropertySemantics semantics = PropertySemantics::Default);

    MarginProperty(const MarginProperty &rhs);
    virtual MarginProperty *clone() const override;

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

}  // namespace plot

}  // namespace inviwo

#endif  // IVW_MARGINPROPERTY_H
