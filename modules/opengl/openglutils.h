/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_OPENGLUTILS_H
#define IVW_OPENGLUTILS_H

#include <modules/opengl/openglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/opengl/glwrap/textureunit.h>

#include <functional>

namespace inviwo {

namespace utilgl {

struct TexParameter {
    TexParameter() = delete;
    TexParameter(TexParameter const&) = delete;
    TexParameter& operator=(TexParameter const& that) = delete;

    TexParameter(const TextureUnit& unit, GLenum target, GLenum name, GLint value)
        : unit_(unit.getEnum()), target_(target), name_(name), oldValue_{} {
        glActiveTexture(unit_);
        glGetTexParameteriv(target_, name_, &oldValue_);
        glTexParameteri(target_, name_, value);
        TextureUnit::setZeroUnit();
    }

    TexParameter(TexParameter&& rhs)
        : unit_(rhs.unit_), target_(rhs.target_), name_(rhs.name_), oldValue_(rhs.oldValue_) {
        rhs.target_ = 0;
    }
    TexParameter& operator=(TexParameter&& that) {
        if (this != &that) {
            unit_ = 0;
            std::swap(unit_, that.unit_);
            target_ = 0;
            std::swap(target_, that.target_);
            name_ = that.name_;
            oldValue_ = that.oldValue_;
        }
        return *this;
    }

    ~TexParameter() {
        if(unit_ != 0 && target_ != 0) {
            glActiveTexture(unit_);
            glTexParameteri(target_, name_, oldValue_);
            TextureUnit::setZeroUnit();
        }
    } 

private:
    GLint unit_;
    GLenum target_;
    GLenum name_;
    int oldValue_;
};

}  // namespace

}  // namespace

#endif  // IVW_OPENGLUTILS_H
