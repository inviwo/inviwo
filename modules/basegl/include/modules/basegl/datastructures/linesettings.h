/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2023 Inviwo Foundation
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
    LineSettings(const LineSettingsInterface* other);
    virtual ~LineSettings() = default;

    float lineWidth = 1.f;
    float antialiasing = 0.5f;
    float miterLimit = 0.8f;
    bool roundCaps = true;
    bool pseudoLighting = false;
    bool roundDepthProfile = false;
    StipplingSettings stippling;
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
    /*
     * @copydoc LineSettingsInterface::getStippling
     */
    virtual const StipplingSettingsInterface& getStippling() const override;
};

}  // namespace inviwo
