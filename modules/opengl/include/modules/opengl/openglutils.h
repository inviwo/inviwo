/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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

#include <modules/opengl/openglmoduledefine.h>  // for IVW_MODULE_OPENGL_API

#include <inviwo/core/datastructures/image/imagetypes.h>  // for Wrapping, InterpolationType
#include <inviwo/core/util/glmvec.h>                      // for vec4, bvec4, ivec4, bvec2, bvec3
#include <inviwo/core/util/moveonlyvalue.h>               // for MoveOnlyValue
#include <inviwo/core/util/stdextensions.h>               // for make_array, index_of
#include <modules/opengl/inviwoopengl.h>                  // for GLint, GLenum, GLboolean, GLAPI...

#include <array>        // for array, operator==
#include <cstddef>      // for size_t
#include <cstdint>      // for int32_t, uint32_t
#include <string_view>  // for basic_string_view, string_view
#include <tuple>        // for tuple

namespace inviwo {
class TextureUnit;

namespace utilgl {

IVW_MODULE_OPENGL_API std::array<GLint, 4> convertSwizzleMaskToGL(const SwizzleMask& mask);
IVW_MODULE_OPENGL_API SwizzleMask convertSwizzleMaskFromGL(const std::array<GLint, 4>& maskGL);

IVW_MODULE_OPENGL_API GLenum convertInterpolationToGL(InterpolationType interpolation);
IVW_MODULE_OPENGL_API InterpolationType convertInterpolationFromGL(GLenum interpolation);

IVW_MODULE_OPENGL_API GLenum convertWrappingToGL(Wrapping wrap);

template <size_t N>
std::array<GLenum, N> convertWrappingToGL(const std::array<Wrapping, N>& wrapping) {
    return util::make_array<N>([&](auto i) { return convertWrappingToGL(wrapping[i]); });
}

IVW_MODULE_OPENGL_API Wrapping convertWrappingFromGL(GLenum wrap);

template <size_t N>
std::array<Wrapping, N> convertWrappingFromGL(const std::array<GLenum, N>& wrapping) {
    return util::make_array<N>([&](auto i) { return convertWrappingFromGL(wrapping[i]); });
}

/**
 * @brief RAII object for texture parameters of a texture bound to a given texture unit
 * @see glTexParameteri
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
 * @brief RAII object for texture environments of a texture bound to a given texture unit
 * @see glTexEnvi
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
 * @brief RAII object for OpenGL bool states
 * @see glIsEnabled, glEnable, glDisable
 */
struct IVW_MODULE_OPENGL_API GlBoolState {
    GlBoolState(GLenum target, bool state);

    GlBoolState() = delete;
    GlBoolState(GlBoolState const&) = delete;
    GlBoolState& operator=(GlBoolState const& that) = delete;
    GlBoolState(GlBoolState&& rhs);
    GlBoolState& operator=(GlBoolState&& that);

    operator bool();

    ~GlBoolState();

protected:
    GLenum target_;
    bool oldState_;
    bool state_;
};

/**
 * @brief RAII object for OpenGL cull face state, which enables GL_CULL_FACE if
 * mode is different from GL_NONE
 * @see glCullFace, GL_CULL_FACE
 */
struct IVW_MODULE_OPENGL_API CullFaceState : protected GlBoolState {
    CullFaceState(GLint mode);

    CullFaceState() = delete;
    CullFaceState(CullFaceState const&) = delete;
    CullFaceState& operator=(CullFaceState const& that) = delete;
    CullFaceState(CullFaceState&& rhs);
    CullFaceState& operator=(CullFaceState&& that);

    ~CullFaceState();

    GLint getMode();

protected:
    GLint mode_;
    GLint oldMode_;
};

/**
 * @brief RAII object for OpenGL polygon mode as well as line width and point size
 * Will set the polygon mode for GL_FRONT_AND_BACK since this is the only mode supported by the
 * OpenGL core profile.
 * @see glPolygonMode, glPointSize, glLineWidth
 */
struct IVW_MODULE_OPENGL_API PolygonModeState {
    PolygonModeState(GLenum mode, GLfloat lineWidth, GLfloat pointSize);

    PolygonModeState() = delete;
    PolygonModeState(PolygonModeState const&) = delete;
    PolygonModeState& operator=(PolygonModeState const& that) = delete;
    PolygonModeState(PolygonModeState&& rhs);
    PolygonModeState& operator=(PolygonModeState&& that);

    ~PolygonModeState();

protected:
    GLint mode_;
    GLfloat lineWidth_;
    GLfloat pointSize_;

    GLint oldMode_;
    GLfloat oldLineWidth_;
    GLfloat oldPointSize_;
};

/**
 * @brief RAII object for OpenGL blending and blend mode, enables blending if source mode is
 * different from GL_NONE
 * @see glBlendFunc, GL_BLEND
 */
struct IVW_MODULE_OPENGL_API BlendModeState : protected GlBoolState {
    BlendModeState(GLenum srcMode, GLenum dstMode);
    BlendModeState(GLenum srcRGB, GLenum srcAlpha, GLenum dstRGB, GLenum dstAlpha);

    BlendModeState() = delete;
    BlendModeState(BlendModeState const&) = delete;
    BlendModeState& operator=(BlendModeState const& that) = delete;
    BlendModeState(BlendModeState&& rhs);
    BlendModeState& operator=(BlendModeState&& that);

    ~BlendModeState();

protected:
    struct Mode {
        GLint rgb;
        GLint alpha;
    };
    struct Config {
        Mode src;
        Mode dst;
    };
    Config curr_;
    Config old_;
};

/**
 * @brief RAII object for OpenGL blend equation
 * @see glBlendEquation, GL_BLEND_EQUATION_RGB
 */
struct IVW_MODULE_OPENGL_API BlendModeEquationState : protected BlendModeState {
    BlendModeEquationState(GLenum srcMode, GLenum dstMode, GLenum eqn);
    BlendModeEquationState(GLenum srcRGB, GLenum srcAlpha, GLenum dstRGB, GLenum dstAlpha,
                           GLenum eqnRGB, GLenum eqnAlpha);

    BlendModeEquationState() = delete;
    BlendModeEquationState(BlendModeEquationState const&) = delete;
    BlendModeEquationState& operator=(BlendModeEquationState const& that) = delete;
    BlendModeEquationState(BlendModeEquationState&& rhs);
    BlendModeEquationState& operator=(BlendModeEquationState&& that);
    ~BlendModeEquationState();

protected:
    struct Equation {
        GLint rgb;
        GLint alpha;
    };

    Equation curr_;
    Equation old_;
};

/**
 * @brief RAII object for OpenGL clear color used when calling glClear(GL_COLOR_BUFFER_BIT)
 * @see glClearColor
 */
struct IVW_MODULE_OPENGL_API ClearColor {
    ClearColor(vec4 color);

    ClearColor() = delete;
    ClearColor(ClearColor const&) = delete;
    ClearColor& operator=(ClearColor const& that) = delete;
    ClearColor(ClearColor&& rhs) = default;
    ClearColor& operator=(ClearColor&& that) = default;
    ~ClearColor();

protected:
    MoveOnlyValue<vec4> oldColor_;
};

/**
 * @brief RAII object for OpenGL clear depth used when calling glClear(GL_DEPTH_BUFFER_BIT)
 * @see glClearDepth
 */
struct IVW_MODULE_OPENGL_API ClearDepth {
    ClearDepth(float depth);

    ClearDepth() = delete;
    ClearDepth(ClearDepth const&) = delete;
    ClearDepth& operator=(ClearDepth const& that) = delete;
    ClearDepth(ClearDepth&& rhs) = default;
    ClearDepth& operator=(ClearDepth&& that) = default;
    ~ClearDepth();

protected:
    MoveOnlyValue<float> oldDepth_;
};

/**
 * @brief representation of an OpenGL viewport with setter and getter
 * @see glViewport, GL_VIEWPORT
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

    friend inline bool operator==(const Viewport& a, const Viewport& b) {
        return a.view_ == b.view_;
    }
    friend inline bool operator!=(const Viewport& lhs, const Viewport& rhs) {
        return !(lhs == rhs);
    }

private:
    std::array<GLint, 4> view_;
};

/**
 * @brief RAII object for OpenGL viewports
 * @see glViewport
 */
struct IVW_MODULE_OPENGL_API ViewportState {
    ViewportState(GLint x, GLint y, GLsizei width, GLsizei height);
    ViewportState(const ivec4& coords);

    ViewportState() = delete;
    ViewportState(ViewportState&&) noexcept = default;
    ViewportState& operator=(ViewportState&&) noexcept = default;
    ~ViewportState();

private:
    MoveOnlyValue<Viewport> oldCoords_;
};

/**
 * @brief representation of an OpenGL viewport with setter and getter
 * @see glScissor, GL_SCISSOR_BOX
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

    friend inline bool operator==(const ScissorBox& a, const ScissorBox& b) {
        return a.box_ == b.box_;
    }
    friend inline bool operator!=(const ScissorBox& lhs, const ScissorBox& rhs) {
        return !(lhs == rhs);
    }

private:
    std::array<GLint, 4> box_;
};

/**
 * @brief RAII object for OpenGL scissor state
 * @see glScissor
 */
struct IVW_MODULE_OPENGL_API ScissorState {
    ScissorState(GLint x, GLint y, GLsizei width, GLsizei height);
    ScissorState(const ivec4& coords);

    ScissorState() = delete;
    ScissorState(ScissorState&&) noexcept = default;
    ScissorState& operator=(ScissorState&&) noexcept = default;
    ~ScissorState();

private:
    MoveOnlyValue<ScissorBox> oldBox_;
};

/**
 * @brief representation of the OpenGL color write mask
 * @see glColorMask, GL_COLOR_WRITE_MASK
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

    friend inline bool operator==(const ColorMask& a, const ColorMask& b) {
        return a.mask_ == b.mask_;
    }
    friend inline bool operator!=(const ColorMask& lhs, const ColorMask& rhs) {
        return !(lhs == rhs);
    }

private:
    std::array<GLboolean, 4> mask_;
};

/**
 * @brief representation of the OpenGL color write mask of a specific buffer
 * @see glColorMaski, GL_COLOR_WRITE_MASK
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

    friend inline bool operator==(const ColorMaski& a, const ColorMaski& b) {
        return a.buf_ == b.buf_ && a.mask_ == b.mask_;
    }
    friend inline bool operator!=(const ColorMaski& lhs, const ColorMaski& rhs) {
        return !(lhs == rhs);
    }

private:
    GLuint buf_;
    std::array<GLboolean, 4> mask_;
};

/**
 * @brief RAII object for OpenGL color mask state, sets the color mask for _all_ draw buffers
 * @see glColorMask
 */
struct IVW_MODULE_OPENGL_API ColorMaskState {
    ColorMaskState(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
    ColorMaskState(const bvec4& mask);

    ColorMaskState() = delete;
    ColorMaskState(ColorMaskState&&) noexcept = default;
    ColorMaskState& operator=(ColorMaskState&&) noexcept = default;
    ~ColorMaskState();

private:
    MoveOnlyValue<ColorMask> oldMask_;
};

/**
 * @brief RAII object for OpenGL color mask state, sets the color mask for a specific draw buffer
 * @see glColorMaski
 */
struct IVW_MODULE_OPENGL_API ColorMaskiState {
    ColorMaskiState(GLuint buf, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
    ColorMaskiState(GLuint buf, const bvec4& mask);

    ColorMaskiState() = delete;
    ColorMaskiState(ColorMaskiState&&) noexcept = default;
    ColorMaskiState& operator=(ColorMaskiState&&) noexcept = default;
    ~ColorMaskiState();

private:
    GLuint buf_;
    MoveOnlyValue<ColorMask> oldMask_;
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
 * @brief RAII object for OpenGL depth func state
 * @see glDepthFunc, GL_DEPTH_FUNC
 */
using DepthFuncState = SimpleState<GLint, GLenum, GL_DEPTH_FUNC, glGetIntegerv, glDepthFunc>;

/**
 * @brief RAII object for OpenGL depth mask to enable/disable writing depth
 * @see glDepthMask, GL_DEPTH_WRITEMASK
 */
using DepthMaskState =
    SimpleState<GLboolean, GLboolean, GL_DEPTH_WRITEMASK, glGetBooleanv, glDepthMask>;

/**
 * @brief RAII object for OpenGL line width
 * @see glLineWidth, GL_LINE_WIDTH
 */

using LineWidthState [[deprecated(
    "glLineWidth is not supported by all OpenGL implementations for widths different from 1.0")]] =
    SimpleState<GLfloat, GLfloat, GL_LINE_WIDTH, glGetFloatv, glLineWidth, validateLineWidth>;

/**
 * @brief RAII object for OpenGL point size
 * @see glPointSize, GL_POINT_SIZE
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

template <typename T>
inline constexpr std::string_view glslTypeName() {
    using types =
        std::tuple<float, double, bool, uint32_t, int32_t, vec2, dvec2, bvec2, ivec2, uvec2, vec3,
                   dvec3, bvec3, ivec3, uvec3, vec4, dvec4, bvec4, ivec4, uvec4>;
    constexpr std::array<std::string_view, 20> names = {
        "float", "double", "bool",  "uint",  "int",   "vec2", "dvec2", "bvec2", "ivec2", "uvec2",
        "vec3",  "dvec3",  "bvec3", "ivec3", "uvec3", "vec4", "dvec4", "bvec4", "ivec4", "uvec4"};

    return names[util::index_of<T, types>()];
}

}  // namespace utilgl

}  // namespace inviwo
