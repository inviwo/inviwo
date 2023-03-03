/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2023 Inviwo Foundation
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
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/util/glmvec.h>

namespace inviwo {

struct IVW_CORE_API SelectionColorState {
    vec4 color;
    float colorMixIn;
    bool visible;
};

/**
 * @ingroup properties
 * @brief composite property holding parameters for highlighted and selected data points.
 */
class IVW_CORE_API SelectionColorProperty : public BoolCompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    SelectionColorProperty(
        std::string_view identifier, std::string_view displayName, bool checked = false,
        vec3 color = vec3(1.0f, 0.906f, 0.612f), float alpha = 0.75f,
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
        PropertySemantics semantics = PropertySemantics::Default);
    SelectionColorProperty(
        std::string_view identifier, std::string_view displayName, Document help,
        bool checked = false, vec3 color = vec3(1.0f, 0.906f, 0.612f), float alpha = 0.75f,
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
        PropertySemantics semantics = PropertySemantics::Default);
    SelectionColorProperty(const SelectionColorProperty& rhs);
    virtual ~SelectionColorProperty() = default;

    virtual SelectionColorProperty* clone() const override;

    SelectionColorState getState() const;

    vec4 getColor() const;
    /**
     * Return the blending factor for mixing the selection color with the item's original color.
     * @return blending factor in [0,1]
     */
    float getMixIntensity() const;

    FloatVec3Property color_;
    FloatProperty alpha_;
    FloatProperty intensity_;
};

}  // namespace inviwo
