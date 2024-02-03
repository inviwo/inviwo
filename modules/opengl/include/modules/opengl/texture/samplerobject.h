/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <modules/opengl/openglmoduledefine.h>

#include <modules/opengl/inviwoopengl.h>
#include <inviwo/core/datastructures/image/imagetypes.h>

namespace inviwo {

/**
 * Sampler objects store parameters that are not specific to textures. Thus, they store filter
 * modes, wrapping mode but not texture width, height, swizzlemask etc. Binding a sampler object to
 * a texture unit takes a texture's uniform location in the shader. Texture units should have only
 * one or zero sampler objects and one texture bound at any time.
 */
class IVW_MODULE_OPENGL_API SamplerObject {
public:
    SamplerObject(InterpolationType type = InterpolationType::Linear,
                  Wrapping3D wrap = {Wrapping::Clamp, Wrapping::Clamp, Wrapping::Clamp});
    SamplerObject(const SamplerObject&) = delete;
    SamplerObject(SamplerObject&&) noexcept;
    SamplerObject& operator=(const SamplerObject&) = delete;
    SamplerObject& operator=(SamplerObject&&) noexcept;
    ~SamplerObject();

    void setMinFilterMode(GLenum interpolation);
    void setMinFilterMode(InterpolationType type);
    void setMagFilterMode(GLenum interpolation);
    void setMagFilterMode(InterpolationType interpolation);
    void setFilterModeAll(GLenum interpolation);
    void setFilterModeAll(InterpolationType interpolation);
    void setWrapMode_S(GLenum wrap);
    void setWrapMode_T(GLenum wrap);
    void setWrapMode_R(GLenum wrap);
    void setWrapModeAll(GLenum wrap);
    void setWrapMode_S(Wrapping wrap);
    void setWrapMode_T(Wrapping wrap);
    void setWrapMode_R(Wrapping wrap);
    void setWrapMode(Wrapping3D wrap);
    GLint getID() const;

private:
    GLuint id_;
};

}  // namespace inviwo
