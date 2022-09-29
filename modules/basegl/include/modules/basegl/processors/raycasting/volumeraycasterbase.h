/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <modules/basegl/processors/shadercomponentprocessorbase.h>

#include <string_view>

namespace inviwo {

/**
 * @brief Base class for volume raycasters.
 * Derived classes should register a set of ShaderComponents to customize behavior
 * This base class uses the "raycasting/raycaster-template.frag" shader template.
 *
 * The following set of placeholders are used by the template:
 * * IVW_SHADER_SEGMENT_PLACEHOLDER_INCLUDE
 * * IVW_SHADER_SEGMENT_PLACEHOLDER_UNIFORM
 * * IVW_SHADER_SEGMENT_PLACEHOLDER_SETUP
 * * IVW_SHADER_SEGMENT_PLACEHOLDER_FIRST
 * * IVW_SHADER_SEGMENT_PLACEHOLDER_LOOP
 * * IVW_SHADER_SEGMENT_PLACEHOLDER_POST
 */
class IVW_MODULE_BASEGL_API VolumeRaycasterBase : public ShaderComponentProcessorBase {
protected:
    VolumeRaycasterBase(std::string_view identifier = "", std::string_view displayName = "");
    virtual ~VolumeRaycasterBase();
};

}  // namespace inviwo
