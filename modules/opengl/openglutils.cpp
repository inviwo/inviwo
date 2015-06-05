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

#include "openglutils.h"

namespace inviwo {
namespace utilgl {

DepthFuncState& utilgl::DepthFuncState::operator=(DepthFuncState&& that) {
    if (this != &that) {
        state_ = that.oldState_;
        std::swap(state_, that.state_);
        oldState_ = that.oldState_;
    }
    return *this;
}

utilgl::DepthFuncState::DepthFuncState(DepthFuncState&& rhs)
    : oldState_(rhs.oldState_), state_(rhs.state_) {
    rhs.state_ = rhs.oldState_;
}

utilgl::DepthFuncState::DepthFuncState(GLenum state) : oldState_{GL_LESS}, state_(state) {
    glGetIntegerv(GL_DEPTH_FUNC, &oldState_);
    if (oldState_ != state_) {
        glDepthFunc(state_);
    }
}

utilgl::DepthFuncState::~DepthFuncState() {
    if (oldState_ != state_) {
        glDepthFunc(oldState_);
    }
}

PolygonModeState& utilgl::PolygonModeState::operator=(PolygonModeState&& that) {
    if (this != &that) {
        mode_ = that.mode_;
        lineWidth_ = that.lineWidth_;
        pointSize_ = that.pointSize_;
        oldMode_ = that.oldMode_;
        oldLineWidth_ = that.oldLineWidth_;
        oldPointSize_ = that.oldPointSize_;

        that.mode_ = GL_NONE;
    }
    return *this;
}

utilgl::PolygonModeState::PolygonModeState(PolygonModeState&& rhs)
    : mode_(rhs.mode_)
    , lineWidth_(rhs.lineWidth_)
    , pointSize_(rhs.pointSize_)
    , oldMode_(rhs.oldMode_)
    , oldLineWidth_(rhs.oldLineWidth_)
    , oldPointSize_(rhs.oldPointSize_) {
    rhs.mode_ = GL_NONE;
}

utilgl::PolygonModeState::PolygonModeState(GLenum mode, GLfloat lineWidth, GLfloat pointSize)
    : mode_(mode)
    , lineWidth_(lineWidth)
    , pointSize_(pointSize)
    , oldMode_(0)
    , oldLineWidth_(0.0f)
    , oldPointSize_(0.0f) {
    // Only GL_FRONT_AND_BACK in core profile.
    glGetIntegerv(GL_POLYGON_MODE, &oldMode_);

    if (static_cast<int>(mode) != oldMode_) glPolygonMode(GL_FRONT_AND_BACK, mode);

    switch (mode_) {
        case GL_POINT: {
            glGetFloatv(GL_POINT_SIZE, &oldPointSize_);
            if (pointSize_ != oldPointSize_) {
                glPointSize(pointSize_);
            }
            break;
        }
        case GL_LINE: {
            glGetFloatv(GL_LINE_WIDTH, &oldLineWidth_);
            if (lineWidth_ != oldLineWidth_) {
                glLineWidth(lineWidth_);
            }
            break;
        }
        case GL_FILL:
        default:
            break;
    }
}

utilgl::PolygonModeState::~PolygonModeState() {
    if (mode_ != GL_NONE) {
        switch (mode_) {
            case GL_POINT: {
                if (pointSize_ != oldPointSize_) {
                    glPointSize(oldPointSize_);
                }
                break;
            }
            case GL_LINE: {
                if (lineWidth_ != oldLineWidth_) {
                    glLineWidth(oldLineWidth_);
                }
                break;
            }
            case GL_FILL:
            default:
                break;
        }
        if (mode_ != oldMode_) glPolygonMode(GL_FRONT_AND_BACK, oldMode_);
    }
}

CullFaceState& utilgl::CullFaceState::operator=(CullFaceState&& that) {
    if (this != &that) {
        GlBoolState::operator=(std::move(that));
        mode_ = that.mode_;
        oldMode_ = that.oldMode_;
        that.mode_ = that.oldMode_;
    }
    return *this;
}

utilgl::CullFaceState::CullFaceState(CullFaceState&& rhs)
    : GlBoolState(std::move(rhs)), mode_(rhs.mode_), oldMode_(rhs.oldMode_) {
    rhs.mode_ = rhs.oldMode_;
}

utilgl::CullFaceState::CullFaceState(GLint mode)
    : GlBoolState(GL_CULL_FACE, mode != GL_NONE), mode_(mode) {
    if (state_) {
        glGetIntegerv(GL_CULL_FACE_MODE, &oldMode_);
        if (oldMode_ != mode) {
            glCullFace(mode);
        }
    }
}

utilgl::CullFaceState::~CullFaceState() {
    if (state_ && oldMode_ != mode_) {
        glCullFace(oldMode_);
    }
}

GLint utilgl::CullFaceState::getMode() { return mode_; }

GlBoolState& utilgl::GlBoolState::operator=(GlBoolState&& that) {
    if (this != &that) {
        target_ = 0;
        std::swap(target_, that.target_);
        state_ = that.oldState_;
        std::swap(state_, that.state_);
        oldState_ = that.oldState_;
    }
    return *this;
}

utilgl::GlBoolState::GlBoolState(GlBoolState&& rhs)
    : target_(rhs.target_), oldState_(rhs.oldState_), state_(rhs.state_) {
    rhs.state_ = rhs.oldState_;
}

utilgl::GlBoolState::GlBoolState(GLenum target, bool state)
    : target_(target), oldState_{}, state_(state) {
    oldState_ = (glIsEnabled(target_) == GL_TRUE);
    if (oldState_ != state_) {
        if (state)
            glEnable(target_);
        else
            glDisable(target_);
    }
}

utilgl::GlBoolState::operator bool() { return state_; }

utilgl::GlBoolState::~GlBoolState() {
    if (oldState_ != state_) {
        if (oldState_)
            glEnable(target_);
        else
            glDisable(target_);
    }
}

TexParameter& utilgl::TexParameter::operator=(TexParameter&& that) {
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

utilgl::TexParameter::TexParameter(TexParameter&& rhs)
    : unit_(rhs.unit_), target_(rhs.target_), name_(rhs.name_), oldValue_(rhs.oldValue_) {
    rhs.target_ = 0;
}

utilgl::TexParameter::TexParameter(const TextureUnit& unit, GLenum target, GLenum name, GLint value)
    : unit_(unit.getEnum()), target_(target), name_(name), oldValue_{} {
    glActiveTexture(unit_);
    glGetTexParameteriv(target_, name_, &oldValue_);
    glTexParameteri(target_, name_, value);
    TextureUnit::setZeroUnit();
}

utilgl::TexParameter::~TexParameter() {
    if (unit_ != 0 && target_ != 0) {
        glActiveTexture(unit_);
        glTexParameteri(target_, name_, oldValue_);
        TextureUnit::setZeroUnit();
    }
}


utilgl::BlendModeState::BlendModeState(GLenum smode, GLenum dmode)
    : GlBoolState(GL_BLEND, smode != GL_NONE), smode_(smode), dmode_(dmode) {
    if (state_) {
        glGetIntegerv(GL_BLEND_SRC, &oldsMode_);
        glGetIntegerv(GL_BLEND_DST, &olddMode_);
        if (oldsMode_ != smode_ || olddMode_ != dmode_) {
            glBlendFunc(smode_, dmode_);
        }
    }
}

BlendModeState& utilgl::BlendModeState::operator=(BlendModeState&& that) {
    if (this != &that) {
        GlBoolState::operator=(std::move(that));
        smode_ = that.smode_;
        oldsMode_ = that.oldsMode_;
        that.smode_ = that.oldsMode_;

        dmode_ = that.dmode_;
        olddMode_ = that.olddMode_;
        that.dmode_ = that.olddMode_;
    }
    return *this;
}

utilgl::BlendModeState::BlendModeState(BlendModeState&& rhs)
    : GlBoolState(std::move(rhs))
    , smode_(rhs.smode_)
    , dmode_(rhs.dmode_)
    , oldsMode_(rhs.oldsMode_)
    , olddMode_(rhs.olddMode_) {
    rhs.smode_ = rhs.oldsMode_;
    rhs.dmode_ = rhs.olddMode_;
}

utilgl::BlendModeState::~BlendModeState() {
    if (state_ && (oldsMode_ != smode_ || olddMode_ != dmode_)) {
        glBlendFunc(oldsMode_, olddMode_);
    }
}

}  // namespace
}  // namespace
