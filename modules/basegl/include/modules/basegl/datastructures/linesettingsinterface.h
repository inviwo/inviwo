/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2020 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>

#include <modules/basegl/properties/stipplingproperty.h>

namespace inviwo {
/*
 * \brief Settings for line rendering
 */
class IVW_MODULE_BASEGL_API LineSettingsInterface {
public:
    LineSettingsInterface() = default;
    virtual ~LineSettingsInterface() = default;
    /*
     * @return Line width (in pixels)
     */
    virtual float getWidth() const = 0;
    /*
     * @return Width of antialiasing (in pixels)
     */
    virtual float getAntialiasingWidth() const = 0;
    /*
     * Where to crop of sharp corners.
     * Occurs when two lines meeting at low angles.
     * @return distance (in pixels)
     */
    virtual float getMiterLimit() const = 0;
    /*
     * Shound line meeting points points be round?
     */
    virtual bool getRoundCaps() const = 0;
    /*
     * Make lines appear cylinder shaped?
     */
    virtual bool getPseudoLighting() const = 0;
    /*
     * Depth values according to cylinder shape?
     */
    virtual bool getRoundDepthProfile() const = 0;
    /*
     * Dashed line settings, e.g., - - -
     */
    virtual const StipplingSettingsInterface& getStippling() const = 0;
};

IVW_MODULE_BASEGL_API bool operator==(const LineSettingsInterface& a,
                                      const LineSettingsInterface& b);
IVW_MODULE_BASEGL_API bool operator!=(const LineSettingsInterface& a,
                                      const LineSettingsInterface& b);

}  // namespace inviwo
