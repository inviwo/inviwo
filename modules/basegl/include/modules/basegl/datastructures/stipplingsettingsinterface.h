/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2024 Inviwo Foundation
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

namespace inviwo {

/*
 * \brief Settings for stippling (Dashed line, e.g., - - -)
 */
class IVW_MODULE_BASEGL_API StipplingSettingsInterface {
public:
    /*
     * \brief Determines in which space the stippling parameters should be applied.
     */
    enum class Mode { None, ScreenSpace, WorldSpace };
    StipplingSettingsInterface() = default;
    virtual ~StipplingSettingsInterface() = default;
    /*
     * Determines which space the other settings should be applied.
     */
    virtual Mode getMode() const = 0;
    /*
     * Return length of dash, in pixels if Mode is ScreenSpace.
     */
    virtual float getLength() const = 0;
    /*
     * Return distance between two dashes, in pixels if Mode is ScreenSpace.
     */
    virtual float getSpacing() const = 0;
    /*
     * Return offset of first dash, in pixels if Mode is ScreenSpace.
     */
    virtual float getOffset() const = 0;
    /*
     * Return scaling of parameters. Only applicable if Mode is WorldSpace.
     */
    virtual float getWorldScale() const = 0;
};

IVW_MODULE_BASEGL_API bool operator==(const StipplingSettingsInterface& a,
                                      const StipplingSettingsInterface& b);
IVW_MODULE_BASEGL_API bool operator!=(const StipplingSettingsInterface& a,
                                      const StipplingSettingsInterface& b);

}  // namespace inviwo
