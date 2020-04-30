/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
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

#include <modules/opengl/openglutils.h>

#include <modules/opengl/openglexception.h>

#include <algorithm>

namespace inviwo {
namespace utilgl {

std::array<GLint, 4> convertSwizzleMaskToGL(const SwizzleMask& mask) {
    auto convertToGL = [](ImageChannel channel) {
        switch (channel) {
            case ImageChannel::Red:
                return GL_RED;
            case ImageChannel::Green:
                return GL_GREEN;
            case ImageChannel::Blue:
                return GL_BLUE;
            case ImageChannel::Alpha:
                return GL_ALPHA;
            case ImageChannel::Zero:
                return GL_ZERO;
            case ImageChannel::One:
                return GL_ONE;
            default:
                return GL_ZERO;
        }
    };
    std::array<GLint, 4> swizzleMaskGL;
    std::transform(mask.begin(), mask.end(), swizzleMaskGL.begin(), convertToGL);
    return swizzleMaskGL;
}

GLenum convertWrappingToGL(Wrapping wrap) {
    switch (wrap) {
        case Wrapping::Clamp:
            return GL_CLAMP_TO_EDGE;
        case Wrapping::Repeat:
            return GL_REPEAT;
        case Wrapping::Mirror:
            return GL_MIRRORED_REPEAT;
        default:
            return GL_CLAMP_TO_EDGE;
    }
}

Wrapping convertWrappingFromGL(GLenum wrap) {
    switch (wrap) {
        case GL_CLAMP_TO_EDGE:
            return Wrapping::Clamp;
        case GL_REPEAT:
            return Wrapping::Repeat;
        case GL_MIRRORED_REPEAT:
            return Wrapping::Mirror;
        default:
            throw OpenGLException("Unsupported Wrapping mode encountered",
                                  IVW_CONTEXT_CUSTOM("convertWrappingFromGL"));
    }
}

GLenum convertInterpolationToGL(InterpolationType interpolation) {
    switch (interpolation) {
        case InterpolationType::Linear:
            return GL_LINEAR;
        case InterpolationType::Nearest:
            return GL_NEAREST;
        default:
            return GL_LINEAR;
    }
}

InterpolationType convertInterpolationFromGL(GLenum interpolation) {
    switch (interpolation) {
        case GL_LINEAR:
            return InterpolationType::Linear;
        case GL_NEAREST:
            return InterpolationType::Nearest;
        default:
            throw OpenGLException("Unsupported filtering mode encountered",
                                  IVW_CONTEXT_CUSTOM("convertInterpolationFromGL"));
    }
}

SwizzleMask convertSwizzleMaskFromGL(const std::array<GLint, 4>& maskGL) {
    auto convertFromGL = [](GLint channel) {
        switch (channel) {
            case GL_RED:
                return ImageChannel::Red;
            case GL_GREEN:
                return ImageChannel::Green;
            case GL_BLUE:
                return ImageChannel::Blue;
            case GL_ALPHA:
                return ImageChannel::Alpha;
            case GL_ZERO:
                return ImageChannel::Zero;
            case GL_ONE:
                return ImageChannel::One;
            default:
                return ImageChannel::Zero;
        }
    };
    SwizzleMask mask;
    std::transform(maskGL.begin(), maskGL.end(), mask.begin(), convertFromGL);
    return mask;
}

PolygonModeState& PolygonModeState::operator=(PolygonModeState&& that) {
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

PolygonModeState::PolygonModeState(PolygonModeState&& rhs)
    : mode_(rhs.mode_)
    , lineWidth_(rhs.lineWidth_)
    , pointSize_(rhs.pointSize_)
    , oldMode_(rhs.oldMode_)
    , oldLineWidth_(rhs.oldLineWidth_)
    , oldPointSize_(rhs.oldPointSize_) {
    rhs.mode_ = GL_NONE;
}

PolygonModeState::PolygonModeState(GLenum mode, GLfloat lineWidth, GLfloat pointSize)
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

PolygonModeState::~PolygonModeState() {
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

CullFaceState& CullFaceState::operator=(CullFaceState&& that) {
    if (this != &that) {
        GlBoolState::operator=(std::move(that));
        mode_ = that.mode_;
        oldMode_ = that.oldMode_;
        that.mode_ = that.oldMode_;
    }
    return *this;
}

CullFaceState::CullFaceState(CullFaceState&& rhs)
    : GlBoolState(std::move(rhs)), mode_(rhs.mode_), oldMode_(rhs.oldMode_) {
    rhs.mode_ = rhs.oldMode_;
}

CullFaceState::CullFaceState(GLint mode) : GlBoolState(GL_CULL_FACE, mode != GL_NONE), mode_(mode) {
    if (state_) {
        glGetIntegerv(GL_CULL_FACE_MODE, &oldMode_);
        if (oldMode_ != mode) {
            glCullFace(mode);
        }
    }
}

CullFaceState::~CullFaceState() {
    if (state_ && oldMode_ != mode_) {
        glCullFace(oldMode_);
    }
}

GLint CullFaceState::getMode() { return mode_; }

GlBoolState& GlBoolState::operator=(GlBoolState&& that) {
    if (this != &that) {
        target_ = 0;
        std::swap(target_, that.target_);
        state_ = that.oldState_;
        std::swap(state_, that.state_);
        oldState_ = that.oldState_;
    }
    return *this;
}

GlBoolState::GlBoolState(GlBoolState&& rhs)
    : target_(rhs.target_), oldState_(rhs.oldState_), state_(rhs.state_) {
    rhs.state_ = rhs.oldState_;
}

GlBoolState::GlBoolState(GLenum target, bool state) : target_(target), oldState_{}, state_(state) {
    oldState_ = (glIsEnabled(target_) == GL_TRUE);
    if (oldState_ != state_) {
        if (state)
            glEnable(target_);
        else
            glDisable(target_);
    }
}

GlBoolState::operator bool() { return state_; }

GlBoolState::~GlBoolState() {
    if (oldState_ != state_) {
        if (oldState_)
            glEnable(target_);
        else
            glDisable(target_);
    }
}

TexParameter& TexParameter::operator=(TexParameter&& that) {
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

TexParameter::TexParameter(TexParameter&& rhs)
    : unit_(rhs.unit_), target_(rhs.target_), name_(rhs.name_), oldValue_(rhs.oldValue_) {
    rhs.target_ = 0;
}

TexParameter::TexParameter(const TextureUnit& unit, GLenum target, GLenum name, GLint value)
    : unit_(unit.getEnum()), target_(target), name_(name), oldValue_{} {
    glActiveTexture(unit_);
    glGetTexParameteriv(target_, name_, &oldValue_);
    glTexParameteri(target_, name_, value);
    TextureUnit::setZeroUnit();
}

TexParameter::~TexParameter() {
    if (unit_ != 0 && target_ != 0) {
        glActiveTexture(unit_);
        glTexParameteri(target_, name_, oldValue_);
        TextureUnit::setZeroUnit();
    }
}

TexEnv& TexEnv::operator=(TexEnv&& that) {
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

TexEnv::TexEnv(TexEnv&& rhs)
    : unit_(rhs.unit_), target_(rhs.target_), name_(rhs.name_), oldValue_(rhs.oldValue_) {
    rhs.target_ = 0;
}

TexEnv::TexEnv(const TextureUnit& unit, GLenum target, GLenum name, GLint value)
    : unit_(unit.getEnum()), target_(target), name_(name), oldValue_{} {
    glGetTexEnviv(target_, name_, &oldValue_);
    glTexEnvi(target_, name_, value);
    TextureUnit::setZeroUnit();
}

TexEnv::~TexEnv() {
    if (unit_ != 0 && target_ != 0) {
        glActiveTexture(unit_);
        glTexEnvi(target_, name_, oldValue_);
        TextureUnit::setZeroUnit();
    }
}

BlendModeState::BlendModeState(GLenum srcRGB, GLenum srcAlpha, GLenum dstRGB, GLenum dstAlpha)
    : GlBoolState(GL_BLEND, srcRGB != GL_NONE || srcAlpha != GL_NONE)
    , curr_{{static_cast<GLint>(srcRGB), static_cast<GLint>(srcAlpha)},
            {static_cast<GLint>(dstRGB), static_cast<GLint>(dstAlpha)}}
    , old_{} {
    if (state_) {
        glGetIntegerv(GL_BLEND_SRC_RGB, &old_.src.rgb);
        glGetIntegerv(GL_BLEND_DST_RGB, &old_.dst.rgb);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &old_.src.alpha);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &old_.dst.alpha);
        if (old_.src.rgb != curr_.src.rgb || old_.dst.rgb != curr_.dst.rgb ||
            old_.src.alpha != curr_.src.alpha || old_.dst.alpha != curr_.dst.alpha) {
            glBlendFuncSeparate(curr_.src.rgb, curr_.dst.rgb, curr_.src.alpha, curr_.dst.alpha);
        }
    }
}

BlendModeState::BlendModeState(GLenum srcMode, GLenum dstMode)
    : BlendModeState{srcMode, srcMode, dstMode, dstMode} {}

BlendModeState& BlendModeState::operator=(BlendModeState&& that) {
    if (this != &that) {
        GlBoolState::operator=(std::move(that));
        curr_ = that.curr_;
        old_ = that.old_;
        that.curr_ = that.old_;
    }
    return *this;
}

BlendModeState::BlendModeState(BlendModeState&& rhs)
    : GlBoolState(std::move(rhs)), curr_(rhs.curr_), old_(rhs.old_) {
    rhs.curr_ = rhs.old_;
}

BlendModeState::~BlendModeState() {
    if (state_ && (old_.src.rgb != curr_.src.rgb || old_.dst.rgb != curr_.dst.rgb ||
                   old_.src.alpha != curr_.src.alpha || old_.dst.alpha != curr_.dst.alpha)) {
        glBlendFuncSeparate(old_.src.rgb, old_.dst.rgb, old_.src.alpha, old_.dst.alpha);
    }
}

BlendModeEquationState::BlendModeEquationState(GLenum srcRGB, GLenum srcAlpha, GLenum dstRGB,
                                               GLenum dstAlpha, GLenum eqnRGB, GLenum eqnAlpha)
    : BlendModeState(srcRGB, srcAlpha, dstRGB, dstAlpha)
    , curr_{static_cast<GLint>(eqnRGB), static_cast<GLint>(eqnAlpha)}
    , old_{} {
    if (state_) {
        glGetIntegerv(GL_BLEND_EQUATION_RGB, &old_.rgb);
        glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &old_.alpha);
    }
    if (old_.rgb != curr_.rgb || old_.alpha != curr_.alpha) {
        glBlendEquationSeparate(curr_.rgb, curr_.alpha);
    }
}

BlendModeEquationState::BlendModeEquationState(GLenum srcMode, GLenum dstMode, GLenum eqn)
    : BlendModeEquationState{srcMode, srcMode, dstMode, dstMode, eqn, eqn} {}

BlendModeEquationState& BlendModeEquationState::operator=(BlendModeEquationState&& that) {
    if (this != &that) {
        BlendModeState::operator=(std::move(that));
        curr_ = that.curr_;
        old_ = that.old_;
        that.curr_ = that.old_;
    }
    return *this;
}

BlendModeEquationState::BlendModeEquationState(BlendModeEquationState&& rhs)
    : BlendModeState(std::move(rhs)), curr_(rhs.curr_), old_(rhs.old_) {
    rhs.curr_ = rhs.old_;
}

BlendModeEquationState::~BlendModeEquationState() {
    if (state_ && (old_.rgb != curr_.rgb || old_.alpha != curr_.alpha)) {
        glBlendEquationSeparate(old_.rgb, old_.alpha);
    }
}

ClearColor::ClearColor(vec4 color) : color_(color) {
    glGetFloatv(GL_COLOR_CLEAR_VALUE, glm::value_ptr(oldColor_));
    if (oldColor_ != color_) {
        glClearColor(color_.x, color_.y, color_.z, color_.w);
    }
}

ClearColor::ClearColor(ClearColor&& rhs) : color_(rhs.color_), oldColor_(rhs.oldColor_) {
    rhs.color_ = rhs.oldColor_;
}

ClearColor& ClearColor::operator=(ClearColor&& that) {
    if (this != &that) {
        color_ = that.color_;
        oldColor_ = that.oldColor_;
        that.color_ = that.oldColor_;
    }
    return *this;
}

ClearColor::~ClearColor() {
    if (oldColor_ != color_) {
        glClearColor(oldColor_.x, oldColor_.y, oldColor_.z, oldColor_.w);
    }
}

ClearDepth::ClearDepth(float depth) : depth_(depth) {
    glGetFloatv(GL_DEPTH_CLEAR_VALUE, &oldDepth_);
    if (oldDepth_ != depth_) {
        glClearDepth(depth_);
    }
}

ClearDepth::ClearDepth(ClearDepth&& rhs) : depth_(rhs.depth_), oldDepth_(rhs.oldDepth_) {
    rhs.depth_ = rhs.oldDepth_;
}

ClearDepth& ClearDepth::operator=(ClearDepth&& that) {
    if (this != &that) {
        depth_ = that.depth_;
        oldDepth_ = that.oldDepth_;
        that.depth_ = that.oldDepth_;
    }
    return *this;
}

ClearDepth::~ClearDepth() {
    if (oldDepth_ != depth_) {
        glClearDepth(oldDepth_);
    }
}

ViewportState& ViewportState::operator=(ViewportState&& that) {
    if (this != &that) {
        coords_ = {0, 0, 0, 0};
        std::swap(coords_, that.coords_);
        oldCoords_ = {0, 0, 0, 0};
        std::swap(oldCoords_, that.oldCoords_);
    }
    return *this;
}

ViewportState::ViewportState(ViewportState&& rhs)
    : coords_(rhs.coords_), oldCoords_(rhs.oldCoords_) {}

ViewportState::ViewportState(GLint x, GLint y, GLsizei width, GLsizei height)
    : coords_{x, y, width, height}, oldCoords_{} {
    oldCoords_.get();
    coords_.set();
}

ViewportState::ViewportState(const ivec4& coords)
    : coords_{coords.x, coords.y, coords.z, coords.w}, oldCoords_{} {
    oldCoords_.get();
    coords_.set();
}

ViewportState::~ViewportState() {
    if (coords_ != oldCoords_) {
        oldCoords_.set();
    }
}

void Viewport::get() { glGetIntegerv(GL_VIEWPORT, view_.data()); }

void Viewport::set() { glViewport(x(), y(), width(), height()); }

ScissorState& ScissorState::operator=(ScissorState&& that) {
    if (this != &that) {
        box_ = {0, 0, 0, 0};
        std::swap(box_, that.box_);
        oldBox_ = {0, 0, 0, 0};
        std::swap(oldBox_, that.oldBox_);
    }
    return *this;
}

ScissorState::ScissorState(ScissorState&& rhs) : box_(rhs.box_), oldBox_(rhs.oldBox_) {}

ScissorState::ScissorState(GLint x, GLint y, GLsizei width, GLsizei height)
    : box_{x, y, width, height}, oldBox_{} {
    oldBox_.get();
    box_.set();
}

ScissorState::ScissorState(const ivec4& coords)
    : box_{coords.x, coords.y, coords.z, coords.w}, oldBox_{} {
    oldBox_.get();
    box_.set();
}

ScissorState::~ScissorState() {
    if (box_ != oldBox_) {
        oldBox_.set();
    }
}

void ScissorBox::get() { glGetIntegerv(GL_SCISSOR_BOX, box_.data()); }

void ScissorBox::set() { glScissor(x(), y(), width(), height()); }

ColorMaskState& ColorMaskState::operator=(ColorMaskState&& that) {
    if (this != &that) {
        mask_ = {GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE};
        std::swap(mask_, that.mask_);
        oldMask_ = {GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE};
        std::swap(oldMask_, that.oldMask_);
    }
    return *this;
}

ColorMaskState::ColorMaskState(ColorMaskState&& rhs) : mask_(rhs.mask_), oldMask_(rhs.oldMask_) {}

ColorMaskState::ColorMaskState(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
    : mask_{red, green, blue, alpha}, oldMask_{} {
    oldMask_.get();
    mask_.set();
}

ColorMaskState::ColorMaskState(const bvec4& mask)
    : mask_{mask.x, mask.y, mask.z, mask.w}, oldMask_{} {
    oldMask_.get();
    mask_.set();
}

ColorMaskState::~ColorMaskState() {
    if (mask_ != oldMask_) {
        oldMask_.set();
    }
}

void ColorMask::get() { glGetBooleanv(GL_COLOR_WRITEMASK, mask_.data()); }

void ColorMask::set() { glColorMask(red(), green(), blue(), alpha()); }

ColorMaskiState& ColorMaskiState::operator=(ColorMaskiState&& that) {
    if (this != &that) {
        buf_ = that.buf_;
        mask_ = {GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE};
        std::swap(mask_, that.mask_);
        oldMask_ = {GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE};
        std::swap(oldMask_, that.oldMask_);
    }
    return *this;
}

ColorMaskiState::ColorMaskiState(ColorMaskiState&& rhs)
    : buf_(rhs.buf_), mask_(rhs.mask_), oldMask_(rhs.oldMask_) {}

ColorMaskiState::ColorMaskiState(GLuint buf, GLboolean red, GLboolean green, GLboolean blue,
                                 GLboolean alpha)
    : buf_{buf}, mask_{red, green, blue, alpha}, oldMask_{} {
    oldMask_.get();
    mask_.set();
}

ColorMaskiState::ColorMaskiState(GLuint buf, const bvec4& mask)
    : buf_{buf}, mask_{mask.x, mask.y, mask.z, mask.w}, oldMask_{} {
    oldMask_.get();
    mask_.set();
}

ColorMaskiState::~ColorMaskiState() {
    if (mask_ != oldMask_) {
        oldMask_.set();
    }
}

void ColorMaski::get() {
    // save the state of all draw buffers
    GLint maxDrawBuffers = 8;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
    std::vector<GLenum> drawBuffers(static_cast<size_t>(maxDrawBuffers), GL_NONE);
    for (int i = 0; i < maxDrawBuffers; ++i) {
        GLint value;
        glGetIntegerv(GL_DRAW_BUFFER0 + i, &value);
        drawBuffers[i] = static_cast<GLenum>(value);
    }

    glDrawBuffer(buf_);
    glGetBooleanv(GL_COLOR_WRITEMASK, mask_.data());

    // restore draw buffers
    glDrawBuffers(maxDrawBuffers, drawBuffers.data());
}

void ColorMaski::set() { glColorMaski(buf_, red(), green(), blue(), alpha()); }

GLfloat validateLineWidth(GLfloat width) {
    float s_sizes[2];
    glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, s_sizes);
    width = std::max(width, s_sizes[0]);
    width = std::min(width, s_sizes[1]);
    return width;
}

}  // namespace utilgl

}  // namespace inviwo
