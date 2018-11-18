/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <modules/opengl/texture/textureunit.h>

#include <functional>
#include <array>

namespace inviwo {

namespace utilgl {

std::array<GLint, 4> IVW_MODULE_OPENGL_API convertSwizzleMaskToGL(const SwizzleMask& mask);

SwizzleMask IVW_MODULE_OPENGL_API convertSwizzleMaskFromGL(const std::array<GLint, 4>& maskGL);

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

struct IVW_MODULE_OPENGL_API TexEnv {
    TexEnv() = delete;
    TexEnv(TexEnv const&) = delete;
    TexEnv& operator=(TexEnv const& that) = delete;

    TexEnv(GLenum target, GLenum name, GLint value);

    TexEnv(TexEnv&& rhs);
    TexEnv& operator=(TexEnv&& that);

    ~TexEnv();

private:
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

struct IVW_MODULE_OPENGL_API BlendModeEquationState : public BlendModeState {
    BlendModeEquationState() = delete;
    BlendModeEquationState(BlendModeEquationState const&) = delete;
    BlendModeEquationState& operator=(BlendModeEquationState const& that) = delete;

    BlendModeEquationState(GLenum smode, GLenum dmode, GLenum eqn);
    BlendModeEquationState(BlendModeEquationState&& rhs);
    BlendModeEquationState& operator=(BlendModeEquationState&& that);

    virtual ~BlendModeEquationState();

protected:
    GLint eqn_;
    GLint oldEqn_;
};

struct IVW_MODULE_OPENGL_API ClearColor {
    ClearColor() = delete;
    ClearColor(ClearColor const&) = delete;
    ClearColor& operator=(ClearColor const& that) = delete;

    ClearColor(vec4 color);
    ClearColor(ClearColor&& rhs);
    ClearColor& operator=(ClearColor&& that);

    virtual ~ClearColor();

protected:
    vec4 color_;
    vec4 oldColor_;
};

struct IVW_MODULE_OPENGL_API Viewport {
    Viewport() : view_({{0, 0, 0, 0}}) {}
    Viewport(GLint x, GLint y, GLsizei width, GLsizei height) : view_({{x, y, width, height}}) {}
    void get();
    void set();

    GLint x() const { return view_[0]; }
    GLint y() const { return view_[1]; }
    GLsizei width() const { return view_[2]; }
    GLsizei height() const { return view_[3]; }

    friend bool IVW_MODULE_OPENGL_API operator==(const Viewport& a, const Viewport& b);

private:
    std::array<GLint, 4> view_;
};

inline bool IVW_MODULE_OPENGL_API operator==(const Viewport& a, const Viewport& b) {
    return a.view_ == b.view_;
}
inline bool IVW_MODULE_OPENGL_API operator!=(const Viewport& lhs, const Viewport& rhs) {
    return !(lhs == rhs);
}

struct IVW_MODULE_OPENGL_API ViewportState {
    ViewportState() = delete;
    ViewportState(ViewportState const&) = delete;
    ViewportState& operator=(ViewportState const& that) = delete;

    ViewportState(GLint x, GLint y, GLsizei width, GLsizei height);
    ViewportState(const ivec4& coords);

    ViewportState(ViewportState&& rhs);
    ViewportState& operator=(ViewportState&& that);

    ~ViewportState();

private:
    Viewport coords_;
    Viewport oldCoords_;
};

struct IVW_MODULE_OPENGL_API ScissorBox {
    ScissorBox() : box_({{0, 0, 0, 0}}) {}
    ScissorBox(GLint x, GLint y, GLsizei width, GLsizei height) : box_({{x, y, width, height}}) {}
    void get();
    void set();

    GLint x() const { return box_[0]; }
    GLint y() const { return box_[1]; }
    GLsizei width() const { return box_[2]; }
    GLsizei height() const { return box_[3]; }

    friend bool IVW_MODULE_OPENGL_API operator==(const ScissorBox& a, const ScissorBox& b);

private:
    std::array<GLint, 4> box_;
};

inline bool IVW_MODULE_OPENGL_API operator==(const ScissorBox& a, const ScissorBox& b) {
    return a.box_ == b.box_;
}
inline bool IVW_MODULE_OPENGL_API operator!=(const ScissorBox& lhs, const ScissorBox& rhs) {
    return !(lhs == rhs);
}

struct IVW_MODULE_OPENGL_API ScissorState {
    ScissorState() = delete;
    ScissorState(ScissorState const&) = delete;
    ScissorState& operator=(ScissorState const& that) = delete;

    ScissorState(GLint x, GLint y, GLsizei width, GLsizei height);
    ScissorState(const ivec4& coords);

    ScissorState(ScissorState&& rhs);
    ScissorState& operator=(ScissorState&& that);

    ~ScissorState();

private:
    ScissorBox box_;
    ScissorBox oldBox_;
};

template <typename T>
T passThrough(T x) {
    return x;
}

template <typename T1, typename T2, GLenum Entity, void(GLAPIENTRY* Getter)(GLenum, T1*),
          void(GLAPIENTRY* Setter)(T2), T1 (*Validator)(T1) = &passThrough<T1>>
struct SimpleState {

    SimpleState() = delete;
    SimpleState(SimpleState<T1, T2, Entity, Getter, Setter, Validator> const&) = delete;
    SimpleState<T1, T2, Entity, Getter, Setter, Validator>& operator=(
        SimpleState<T1, T2, Entity, Getter, Setter, Validator> const& that) = delete;

    SimpleState(T1 value);
    SimpleState(SimpleState<T1, T2, Entity, Getter, Setter, Validator>&& rhs);
    SimpleState<T1, T2, Entity, Getter, Setter, Validator>& operator=(
        SimpleState<T1, T2, Entity, Getter, Setter, Validator>&& that);

    operator T1();
    virtual ~SimpleState();

protected:
    T1 oldState_;
    T1 state_;
};

template <typename T1, typename T2, GLenum Entity, void(GLAPIENTRY* Getter)(GLenum, T1*),
          void(GLAPIENTRY* Setter)(T2), T1 (*Validator)(T1)>
SimpleState<T1, T2, Entity, Getter, Setter, Validator>::SimpleState(T1 value)
    : state_(Validator(value)) {
    Getter(Entity, &oldState_);
    if (oldState_ != state_) {
        Setter(state_);
    }
}

template <typename T1, typename T2, GLenum Entity, void(GLAPIENTRY* Getter)(GLenum, T1*),
          void(GLAPIENTRY* Setter)(T2), T1 (*Validator)(T1)>
SimpleState<T1, T2, Entity, Getter, Setter, Validator>::~SimpleState() {
    if (state_ != oldState_) {
        Setter(static_cast<T2>(oldState_));
    }
}

template <typename T1, typename T2, GLenum Entity, void(GLAPIENTRY* Getter)(GLenum, T1*),
          void(GLAPIENTRY* Setter)(T2), T1 (*Validator)(T1)>
SimpleState<T1, T2, Entity, Getter, Setter, Validator>::operator T1() {
    return state_;
}

template <typename T1, typename T2, GLenum Entity, void(GLAPIENTRY* Getter)(GLenum, T1*),
          void(GLAPIENTRY* Setter)(T2), T1 (*Validator)(T1)>
SimpleState<T1, T2, Entity, Getter, Setter, Validator>&
SimpleState<T1, T2, Entity, Getter, Setter, Validator>::operator=(
    SimpleState<T1, T2, Entity, Getter, Setter, Validator>&& that) {
    if (this != &that) {
        state_ = that.oldState_;
        std::swap(state_, that.state_);
        oldState_ = that.oldState_;
    }
    return *this;
}
template <typename T1, typename T2, GLenum Entity, void(GLAPIENTRY* Getter)(GLenum, T1*),
          void(GLAPIENTRY* Setter)(T2), T1 (*Validator)(T1)>
SimpleState<T1, T2, Entity, Getter, Setter, Validator>::SimpleState(
    SimpleState<T1, T2, Entity, Getter, Setter, Validator>&& rhs)
    : oldState_(rhs.oldState_), state_(rhs.state_) {
    rhs.state_ = rhs.oldState_;
}

IVW_MODULE_OPENGL_API GLfloat validateLineWidth(GLfloat width);

using DepthFuncState = SimpleState<GLint, GLenum, GL_DEPTH_FUNC, glGetIntegerv, glDepthFunc>;
using DepthMaskState =
    SimpleState<GLboolean, GLboolean, GL_DEPTH_WRITEMASK, glGetBooleanv, glDepthMask>;
using LineWidthState =
    SimpleState<GLfloat, GLfloat, GL_LINE_WIDTH, glGetFloatv, glLineWidth, validateLineWidth>;
using PointSizeState = SimpleState<GLfloat, GLfloat, GL_POINT_SIZE, glGetFloatv, glPointSize>;

template <typename T>
// requires can enable/disable const
struct Enable {
    Enable(const T* item) : item_(item) { item->enable(); }
    Enable(const Enable&) = delete;
    Enable(Enable&& rhs) : item_{rhs.item_} { rhs.item_ = nullptr; };
    Enable& operator=(const Enable&) = delete;
    Enable& operator=(Enable&& that) {
        if (this != &that) {
            std::swap(item_, that.item_);
            if (that.item_) {
                that.item_->disable();
                that.item_ = nullptr;
            }
        }
        return *this;
    };
    ~Enable() {
        if (item_) item_->disable();
    }

private:
    const T* item_;
};

template <typename T>
// requires can activate/deactivate
struct Activate {
    Activate(T* item) : item_(item) { item->activate(); }
    Activate(const Activate&) = delete;
    Activate(Activate&& rhs) : item_{rhs.item_} { rhs.item_ = nullptr; };
    Activate& operator=(const Activate&) = delete;
    Activate& operator=(Activate&& that) {
        if (this != &that) {
            std::swap(item_, that.item_);
            if (that.item_) {
                that.item_->deactivate();
                that.item_ = nullptr;
            }
        }
        return *this;
    };
    ~Activate() {
        if (item_) item_->deactivate();
    }

private:
    T* item_;
};

}  // namespace utilgl

}  // namespace inviwo

#endif  // IVW_OPENGLUTILS_H
