/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/invalidationlevel.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <modules/basegl/datastructures/linesettingsinterface.h>
#include <modules/basegl/properties/stipplingproperty.h>

#include <string>
#include <string_view>

namespace inviwo {
class StipplingSettingsInterface;

class IVW_MODULE_BASEGL_API LineSettingsProperty : public LineSettingsInterface,
                                                   public CompositeProperty {
public:
    virtual std::string_view getClassIdentifier() const override;
    static const std::string classIdentifier;

    LineSettingsProperty(std::string_view identifier, std::string_view displayName,
                         InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                         PropertySemantics semantics = PropertySemantics::Default);
    LineSettingsProperty(const LineSettingsProperty& rhs);
    virtual ~LineSettingsProperty() = default;

    virtual LineSettingsProperty* clone() const override;

    // Inherited from LineSettingsInterface
    /**
     * @copydoc LineSettingsInterface::getWidth
     */
    virtual float getWidth() const override;
    /**
     * @copydoc LineSettingsInterface::getAntialiasingWidth
     */
    virtual float getAntialiasingWidth() const override;
    /**
     * @copydoc LineSettingsInterface::getMiterLimit
     */
    virtual float getMiterLimit() const override;
    /**
     * @copydoc LineSettingsInterface::getRoundCaps
     */
    virtual bool getRoundCaps() const override;
    /**
     * @copydoc LineSettingsInterface::getPseudoLighting
     */
    virtual bool getPseudoLighting() const override;
    /**
     * @copydoc LineSettingsInterface::getRoundDepthProfile
     */
    virtual bool getRoundDepthProfile() const override;
    /**
     * @copydoc LineSettingsInterface::getDefaultColor
     */
    virtual vec4 getDefaultColor() const override;
    /**
     * @copydoc LineSettingsInterface::getStippling
     */
    virtual const StipplingSettingsInterface& getStippling() const override;

    virtual bool getOverrideColor() const override;
    virtual vec3 getOverrideColorValue() const override;

    virtual bool getOverrideAlpha() const override;
    virtual float getOverrideAlphaValue() const override;

    virtual bool getUseMetaColor() const override;

    virtual const TransferFunction& getMetaColor() const override;

    FloatProperty lineWidth;
    FloatProperty antialiasing;
    FloatProperty miterLimit;
    BoolProperty roundCaps;

    BoolProperty pseudoLighting;
    BoolProperty roundDepthProfile;

    FloatVec4Property defaultColor;
    BoolCompositeProperty overrideColor;
    FloatVec3Property color;
    BoolCompositeProperty overrideAlpha;
    FloatProperty alpha;
    BoolCompositeProperty useMetaColor;
    TransferFunctionProperty metaColor;

    StipplingProperty stippling;
};

}  // namespace inviwo
