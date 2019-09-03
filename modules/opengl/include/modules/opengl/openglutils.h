/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

/**
 * \struct TexParameter
 * \brief RAII object for texture parameters of a texture bound to a given texture unit
 *
 * \see glTexParameteri
 */
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

/**
 * \struct TexEnv
 * \brief RAII object for texture environments of a texture bound to a given texture unit
 *
 * \see glTexEnvi
 */
struct IVW_MODULE_OPENGL_API TexEnv {
    TexEnv() = delete;
    TexEnv(TexEnv const&) = delete;
    TexEnv& operator=(TexEnv const& that) = delete;

    TexEnv(const TextureUnit& unit, GLenum target, GLenum name, GLint value);

    TexEnv(TexEnv&& rhs);
    TexEnv& operator=(TexEnv&& that);

    ~TexEnv();

private:
    GLint unit_;
    GLenum target_;
    GLenum name_;
    int oldValue_;
};

/**
 * \struct GlBoolState
 * \brief RAII object for OpenGL bool states
 *
 * \see glIsEnabled, glEnable, glDisable
 */
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

/**
 * \struct CullFaceState
 * \brief RAII object for OpenGL cull face state, which enables GL_CULL_FACE if
 * mode is different from GL_NONE
 *
 * \see glCullFace, GL_CULL_FACE
 */
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

/**
 * \struct CullFaceState
 * \brief RAII object for OpenGL polygon mode as well as line width and point size
 * Will set the polygon mode for GL_FRONT_AND_BACK since this is the only mode supported by the
 * OpenGL core profile.
 *
 * \see glPolygonMode, glPointSize, glLineWidth
 */
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

/**
 * \struct BlendModeState
 * \brief RAII object for OpenGL blending and blend mode, enables blending if source mode is
 * different from GL_NONE
 *
 * \see glBlendFunc, GL_BLEND
 */
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

/**
 * \struct BlendModeEquationState
 * \brief RAII object for OpenGL blend equation
 *
 * \see glBlendEquation, GL_BLEND_EQUATION_RGB
 */
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

/**
 * \struct ClearColor
 * \brief RAII object for OpenGL clear color used when calling glClear(GL_COLOR_BUFFER_BIT)
 *
 * \see glClearColor
 */
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

/**
 * \struct ClearColor
 * \brief RAII object for OpenGL clear depth used when calling glClear(GL_DEPTH_BUFFER_BIT)
 *
 * \see glClearDepth
 */
struct IVW_MODULE_OPENGL_API ClearDepth {
    ClearDepth() = delete;
    ClearDepth(ClearDepth const&) = delete;
    ClearDepth& operator=(ClearDepth const& that) = delete;

    ClearDepth(float depth);
    ClearDepth(ClearDepth&& rhs);
    ClearDepth& operator=(ClearDepth&& that);

    virtual ~ClearDepth();

protected:
    float depth_;
    float oldDepth_;
};

/**
 * \struct Viewport
 * \brief representation of an OpenGL viewport with setter and getter
 *
 * \see glViewport, GL_VIEWPORT
 */
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

/**
 * \struct ClearColor
 * \brief RAII object for OpenGL viewports
 *
 * \see glViewport
 */
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

/**
 * \struct ScissorBox
 * \brief representation of an OpenGL viewport with setter and getter
 *
 * \see glScissor, GL_SCISSOR_BOX
 */
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

/**
 * \struct ScissorState
 * \brief RAII object for OpenGL scissor state
 *
 * \see glScissor
 */
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

/**
 * \struct ColorMask
 * \brief representation of the OpenGL color write mask
 *
 * \see glColorMask, GL_COLOR_WRITE_MASK
 */
struct IVW_MODULE_OPENGL_API ColorMask {
    ColorMask() : mask_({{0, 0, 0, 0}}) {}
    ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
        : mask_({{red, green, blue, alpha}}) {}
    void get();
    void set();

    GLboolean red() const { return mask_[0]; }
    GLboolean green() const { return mask_[1]; }
    GLboolean blue() const { return mask_[2]; }
    GLboolean alpha() const { return mask_[3]; }

    friend bool IVW_MODULE_OPENGL_API operator==(const ColorMask& a, const ColorMask& b);

private:
    std::array<GLboolean, 4> mask_;
};

inline bool IVW_MODULE_OPENGL_API operator==(const ColorMask& a, const ColorMask& b) {
    return a.mask_ == b.mask_;
}
inline bool IVW_MODULE_OPENGL_API operator!=(const ColorMask& lhs, const ColorMask& rhs) {
    return !(lhs == rhs);
}

/**
 * \struct ColorMaski
 * \brief representation of the OpenGL color write mask of a specific buffer
 *
 * \see glColorMaski, GL_COLOR_WRITE_MASK
 */
struct IVW_MODULE_OPENGL_API ColorMaski {
    ColorMaski() : buf_(0), mask_({{0, 0, 0, 0}}) {}
    ColorMaski(GLuint buf, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
        : buf_(buf), mask_({{red, green, blue, alpha}}) {}
    void get();
    void set();

    GLuint buffer() const { return buf_; }
    GLboolean red() const { return mask_[0]; }
    GLboolean green() const { return mask_[1]; }
    GLboolean blue() const { return mask_[2]; }
    GLboolean alpha() const { return mask_[3]; }

    friend bool IVW_MODULE_OPENGL_API operator==(const ColorMaski& a, const ColorMaski& b);

private:
    GLuint buf_;
    std::array<GLboolean, 4> mask_;
};

inline bool IVW_MODULE_OPENGL_API operator==(const ColorMaski& a, const ColorMaski& b) {
    return a.buf_ == b.buf_ && a.mask_ == b.mask_;
}
inline bool IVW_MODULE_OPENGL_API operator!=(const ColorMaski& lhs, const ColorMaski& rhs) {
    return !(lhs == rhs);
}

/**
 * \struct ColorMaskState
 * \brief RAII object for OpenGL color mask state, sets the color mask for _all_ draw buffers
 *
 * \see glColorMask
 */
struct IVW_MODULE_OPENGL_API ColorMaskState {
    ColorMaskState() = delete;
    ColorMaskState(ColorMaskState const&) = delete;
    ColorMaskState& operator=(ColorMaskState const& that) = delete;

    ColorMaskState(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
    ColorMaskState(const bvec4& mask);

    ColorMaskState(ColorMaskState&& rhs);
    ColorMaskState& operator=(ColorMaskState&& that);

    ~ColorMaskState();

private:
    ColorMask mask_;
    ColorMask oldMask_;
};

/**
 * \struct ColorMaskiState
 * \brief RAII object for OpenGL color mask state, sets the color mask for a specific draw buffer
 *
 * \see glColorMaski
 */
struct IVW_MODULE_OPENGL_API ColorMaskiState {
    ColorMaskiState() = delete;
    ColorMaskiState(ColorMaskiState const&) = delete;
    ColorMaskiState& operator=(ColorMaskiState const& that) = delete;

    ColorMaskiState(GLuint buf, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
    ColorMaskiState(GLuint buf, const bvec4& mask);

    ColorMaskiState(ColorMaskiState&& rhs);
    ColorMaskiState& operator=(ColorMaskiState&& that);

    ~ColorMaskiState();

private:
    GLuint buf_;
    ColorMask mask_;
    ColorMask oldMask_;
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

/**
 * \struct inviwo::DepthFuncState
 * \brief RAII object for OpenGL depth func state
 *
 * \see glDepthFunc, GL_DEPTH_FUNC
 */
using DepthFuncState = SimpleState<GLint, GLenum, GL_DEPTH_FUNC, glGetIntegerv, glDepthFunc>;

/**
 * \struct inviwo::DepthMaskState
 * \brief RAII object for OpenGL depth mask to enable/disable writing depth
 *
 * \see glDepthMask, GL_DEPTH_WRITEMASK
 */
using DepthMaskState =
    SimpleState<GLboolean, GLboolean, GL_DEPTH_WRITEMASK, glGetBooleanv, glDepthMask>;

/**
 * \struct inviwo::LineWidthState
 * \brief RAII object for OpenGL line width
 *
 * \see glLineWidth, GL_LINE_WIDTH
 */

using LineWidthState[[deprecated(
    "glLineWidth is not supported by all OpenGL implementations for widths different from 1.0")]] =
    SimpleState<GLfloat, GLfloat, GL_LINE_WIDTH, glGetFloatv, glLineWidth, validateLineWidth>;

/**
 * \struct inviwo::PointSizeState
 * \brief RAII object for OpenGL point size
 *
 * \see glPointSize, GL_POINT_SIZE
 */
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
