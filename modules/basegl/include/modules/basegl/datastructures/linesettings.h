/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2025 Inviwo Foundation
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

#include <modules/basegl/datastructures/linesettingsinterface.h>  // for LineSettingsInterface
#include <modules/basegl/datastructures/stipplingsettings.h>      // for StipplingSettings
#include <inviwo/core/datastructures/transferfunction.h>

namespace inviwo {
class StipplingSettingsInterface;

/**
 * \brief Basic implementation of the LineSettingsInterface
 */
class IVW_MODULE_BASEGL_API LineSettings : public LineSettingsInterface {
public:
    LineSettings() = default;
    /*
     * Copy settings of other.
     */
    explicit LineSettings(const LineSettingsInterface* other);
    virtual ~LineSettings() = default;

    float lineWidth = 1.f;
    float antialiasing = 0.5f;
    float miterLimit = 0.8f;
    bool roundCaps = true;
    bool pseudoLighting = false;
    bool roundDepthProfile = false;
    bool overrideColor = false;
    bool overrideAlpha = false;
    bool useMetaColor = false;
    StipplingSettings stippling;
    vec4 defaultColor = vec4{1.0f, 0.7f, 0.2f, 1.0f};
    vec3 overrideColorValue = vec3{0.7f, 0.7f, 0.7f};
    float overrideAlphaValue = 1.0f;
    TransferFunction metaColor{
        {{0.0, vec4{0.0f, 0.0f, 0.0f, 0.0f}}, {1.0, vec4{1.0f, 1.0f, 1.0f, 1.0f}}}};

    // Inherited from LineSettingsInterface
    /*
     * @copydoc LineSettingsInterface::getWidth
     */
    virtual float getWidth() const override;
    /*
     * @copydoc LineSettingsInterface::getAntialiasingWidth
     */
    virtual float getAntialiasingWidth() const override;
    /*
     * @copydoc LineSettingsInterface::getMiterLimit
     */
    virtual float getMiterLimit() const override;
    /*
     * @copydoc LineSettingsInterface::getRoundCaps
     */
    virtual bool getRoundCaps() const override;
    /*
     * @copydoc LineSettingsInterface::getPseudoLighting
     */
    virtual bool getPseudoLighting() const override;
    /*
     * @copydoc LineSettingsInterface::getRoundDepthProfile
     */
    virtual bool getRoundDepthProfile() const override;
    /**
     * @copydoc LineSettingsInterface::getDefaultColor
     */
    virtual vec4 getDefaultColor() const override;
    /*
     * @copydoc LineSettingsInterface::getStippling
     */
    virtual const StipplingSettingsInterface& getStippling() const override;

    virtual bool getOverrideColor() const override;
    virtual vec3 getOverrideColorValue() const override;

    virtual bool getOverrideAlpha() const override;
    virtual float getOverrideAlphaValue() const override;

    virtual bool getUseMetaColor() const override;
    virtual const TransferFunction& getMetaColor() const override;
};

}  // namespace inviwo
