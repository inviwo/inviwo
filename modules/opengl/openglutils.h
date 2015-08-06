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
#include <modules/opengl/texture/textureunit.h>

#include <functional>

namespace inviwo {

namespace utilgl {

struct IVW_MODULE_OPENGL_API TexParameter {
    TexParameter() = delete;
    TexParameter(TexParameter const&) = delete;
    TexParameter& operator=(TexParameter const& that) = delete;

    TexParameter(const TextureUnit& unit, GLenum target, GLenum name, GLint value);

    TexParameter(TexParameter&& rhs);
    TexParameter& operator=(TexParameter&& that);

    ~TexParameter();

private:
    GLint unit_;
    GLenum target_;
    GLenum name_;
    int oldValue_;
};

struct IVW_MODULE_OPENGL_API GlBoolState {
    GlBoolState() = delete;
    GlBoolState(GlBoolState const&) = delete;
    GlBoolState& operator=(GlBoolState const& that) = delete;

    GlBoolState(GLenum target, bool state);

    GlBoolState(GlBoolState&& rhs);
    GlBoolState& operator=(GlBoolState&& that);

    operator bool();

    virtual ~GlBoolState();

protected:
    GLenum target_;
    bool oldState_;
    bool state_;
};

struct IVW_MODULE_OPENGL_API CullFaceState : public GlBoolState {
    CullFaceState() = delete;
    CullFaceState(CullFaceState const&) = delete;
    CullFaceState& operator=(CullFaceState const& that) = delete;

    CullFaceState(GLint mode);

    CullFaceState(CullFaceState&& rhs);

    CullFaceState& operator=(CullFaceState&& that);

    virtual ~CullFaceState();

    GLint getMode();

protected:
    GLint mode_;
    GLint oldMode_;
};

struct IVW_MODULE_OPENGL_API PolygonModeState {
    PolygonModeState() = delete;
    PolygonModeState(PolygonModeState const&) = delete;
    PolygonModeState& operator=(PolygonModeState const& that) = delete;

    PolygonModeState(GLenum mode, GLfloat lineWidth, GLfloat pointSize);

    PolygonModeState(PolygonModeState&& rhs);

    PolygonModeState& operator=(PolygonModeState&& that);

    virtual ~PolygonModeState();

protected:
    GLint mode_;
    GLfloat lineWidth_;
    GLfloat pointSize_;

    GLint oldMode_;
    GLfloat oldLineWidth_;
    GLfloat oldPointSize_;
};

struct IVW_MODULE_OPENGL_API BlendModeState : public GlBoolState {
    BlendModeState() = delete;
    BlendModeState(BlendModeState const&) = delete;
    BlendModeState& operator=(BlendModeState const& that) = delete;

    BlendModeState(GLenum smode, GLenum dmode);
    BlendModeState(BlendModeState&& rhs);
    BlendModeState& operator=(BlendModeState&& that);

    virtual ~BlendModeState();

protected:
    GLint smode_;
    GLint dmode_;
    GLint oldsMode_;
    GLint olddMode_;
};

template <typename T1, typename T2, GLenum Entity, void (*Getter)(GLenum, T1*),
          void (*Setter)(T2)>
struct SimpleState {
    SimpleState() = delete;
    SimpleState(SimpleState<T1, T2, Entity, Getter, Setter> const&) = delete;
    SimpleState<T1, T2, Entity, Getter, Setter>& operator=(
        SimpleState<T1, T2, Entity, Getter, Setter> const& that) = delete;

    SimpleState(T1 value);
    SimpleState(SimpleState<T1, T2, Entity, Getter, Setter>&& rhs);
    SimpleState<T1, T2, Entity, Getter, Setter>& operator=(
        SimpleState<T1, T2, Entity, Getter, Setter>&& that);

    operator T1();
    virtual ~SimpleState();

protected:
    T1 oldState_;
    T1 state_;
};

template <typename T1, typename T2, GLenum Entity, void (*Getter)(GLenum, T1*), void (*Setter)(T2)>
SimpleState<T1, T2, Entity, Getter, Setter>::SimpleState(T1 value)
    : state_(value) {
    Getter(Entity, &oldState_);
    if (oldState_ != state_) {
        Setter(state_);
    }
}

template <typename T1, typename T2, GLenum Entity, void (*Getter)(GLenum, T1*), void (*Setter)(T2)>
SimpleState<T1, T2, Entity, Getter, Setter>::~SimpleState() {
    if (state_ != oldState_) {
        Setter(static_cast<T2>(oldState_));
    }
}

template <typename T1, typename T2, GLenum Entity, void (*Getter)(GLenum, T1*), void (*Setter)(T2)>
SimpleState<T1, T2, Entity, Getter, Setter>::operator T1() {
    return state_;
}

template <typename T1, typename T2, GLenum Entity, void (*Getter)(GLenum, T1*), void (*Setter)(T2)>
SimpleState<T1, T2, Entity, Getter, Setter>& SimpleState<T1, T2, Entity, Getter, Setter>::operator=(
    SimpleState<T1, T2, Entity, Getter, Setter>&& that) {
    if (this != &that) {
        state_ = that.oldState_;
        std::swap(state_, that.state_);
        oldState_ = that.oldState_;
    }
    return *this;
}
template <typename T1, typename T2, GLenum Entity, void (*Getter)(GLenum, T1*), void (*Setter)(T2)>
SimpleState<T1, T2, Entity, Getter, Setter>::SimpleState(
    SimpleState<T1, T2, Entity, Getter, Setter>&& rhs)
    : oldState_(rhs.oldState_), state_(rhs.state_) {
    rhs.state_ = rhs.oldState_;
}

using DepthFuncState = SimpleState<GLint, GLenum, GL_DEPTH_FUNC, glGetIntegerv, glDepthFunc>;
using DepthMaskState = SimpleState<GLboolean, GLboolean, GL_DEPTH_WRITEMASK, glGetBooleanv, glDepthMask>;
using LineWidthState = SimpleState<GLfloat, GLfloat, GL_LINE_WIDTH, glGetFloatv, glLineWidth>;
using PointSizeState = SimpleState<GLfloat, GLfloat, GL_POINT_SIZE, glGetFloatv, glPointSize>;

}  // namespace

}  // namespace

#endif  // IVW_OPENGLUTILS_H
