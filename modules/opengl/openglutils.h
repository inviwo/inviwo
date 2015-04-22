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
        if (unit_ != 0 && target_ != 0) {
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

struct GlBoolState {
    GlBoolState() = delete;
    GlBoolState(GlBoolState const&) = delete;
    GlBoolState& operator=(GlBoolState const& that) = delete;

    GlBoolState(GLenum target, bool state) : target_(target), oldState_{}, state_(state) {
        oldState_ = glIsEnabled(target_);
        if (oldState_ != state_) {
            if (state)
                glEnable(target_);
            else
                glDisable(target_);
        }
    }

    GlBoolState(GlBoolState&& rhs)
        : target_(rhs.target_), oldState_(rhs.oldState_), state_(rhs.state_) {
        rhs.state_ = rhs.oldState_;
    }
    GlBoolState& operator=(GlBoolState&& that) {
        if (this != &that) {
            target_ = 0;
            std::swap(target_, that.target_);
            state_ = that.oldState_;
            std::swap(state_, that.state_);
            oldState_ = that.oldState_;
        }
        return *this;
    }

    operator bool() { return state_; };

    virtual ~GlBoolState() {
        if (oldState_ != state_) {
            if (oldState_)
                glEnable(target_);
            else
                glDisable(target_);
        }
    }

protected:
    GLenum target_;
    bool oldState_;
    bool state_;
};

struct CullFaceState : public GlBoolState {
    CullFaceState() = delete;
    CullFaceState(CullFaceState const&) = delete;
    CullFaceState& operator=(CullFaceState const& that) = delete;

    CullFaceState(GLint mode) : GlBoolState(GL_CULL_FACE, mode != GL_NONE), mode_(mode) {
        if (state_) {
            glGetIntegerv(GL_CULL_FACE_MODE, &oldMode_);
            if (oldMode_ != mode) {
                glCullFace(mode);
            }
        }
    }

    CullFaceState(CullFaceState&& rhs)
        : GlBoolState(std::move(rhs)), mode_(rhs.mode_), oldMode_(rhs.oldMode_) {
        rhs.mode_ = rhs.oldMode_;
    }

    CullFaceState& operator=(CullFaceState&& that) {
        if (this != &that) {
            GlBoolState::operator=(std::move(that));
            mode_ = that.mode_;
            oldMode_ = that.oldMode_;
            that.mode_ = that.oldMode_;
        }
        return *this;
    }

    virtual ~CullFaceState() {
        if (state_ && oldMode_ != mode_) {
            glCullFace(oldMode_);
        }
    }

    GLint getMode() { return mode_; }

protected:
    GLint mode_;
    GLint oldMode_;
};

struct PolygonModeState {
    PolygonModeState() = delete;
    PolygonModeState(PolygonModeState const&) = delete;
    PolygonModeState& operator=(PolygonModeState const& that) = delete;

    PolygonModeState(GLenum mode, GLfloat lineWidth, GLfloat pointSize)
        : mode_(mode)
        , lineWidth_(lineWidth)
        , pointSize_(pointSize)
        , oldMode_(0)
        , oldLineWidth_(0.0f)
        , oldPointSize_(0.0f) {
        // Only GL_FRONT_AND_BACK in core profile.
        glGetIntegerv(GL_POLYGON_MODE, glm::value_ptr(oldMode_));

        if (mode != oldMode_[0]) glPolygonMode(GL_FRONT_AND_BACK, mode);

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

    PolygonModeState(PolygonModeState&& rhs)
        : mode_(rhs.mode_)
        , lineWidth_(rhs.lineWidth_)
        , pointSize_(rhs.pointSize_)
        , oldMode_(rhs.oldMode_)
        , oldLineWidth_(rhs.oldLineWidth_)
        , oldPointSize_(rhs.oldPointSize_) {
        rhs.mode_ = GL_NONE;
    }

    PolygonModeState& operator=(PolygonModeState&& that) {
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

    virtual ~PolygonModeState() {
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
            if (mode_ != oldMode_[0]) glPolygonMode(GL_FRONT_AND_BACK, oldMode_[0]);
        }
    }

protected:
    GLint mode_;
    GLfloat lineWidth_;
    GLfloat pointSize_;

    ivec2 oldMode_;
    GLfloat oldLineWidth_;
    GLfloat oldPointSize_;
};

}  // namespace

}  // namespace

#endif  // IVW_OPENGLUTILS_H
