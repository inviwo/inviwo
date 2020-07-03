/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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
#include <modules/basegl/datastructures/stipplingsettingsinterface.h>

namespace inviwo {

/**
 * \brief Basic implementation of the StipplingSettingsInterface
 */
class IVW_MODULE_BASEGL_API StipplingSettings : public StipplingSettingsInterface {
public:
    StipplingSettings() = default;
    StipplingSettings(const StipplingSettingsInterface* other);
    virtual ~StipplingSettings() = default;

    Mode mode = Mode::None;
    float length = 30.f;
    float spacing = 10.f;
    float offset = 0.f;
    float worldScale = 4.f;
    /*
     * @copydoc StipplingSettingsInterface::getMode
     */
    virtual StipplingSettingsInterface::Mode getMode() const override;
    /*
     * @copydoc StipplingSettingsInterface::getLength
     */
    virtual float getLength() const override;
    /*
     * @copydoc StipplingSettingsInterface::getSpacing
     */
    virtual float getSpacing() const override;
    /*
     * @copydoc StipplingSettingsInterface::getOffset
     */
    virtual float getOffset() const override;
    /*
     * @copydoc StipplingSettingsInterface::getWorldScale
     */
    virtual float getWorldScale() const override;
};

}  // namespace inviwo
