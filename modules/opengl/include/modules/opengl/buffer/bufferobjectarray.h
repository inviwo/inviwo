/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2021 Inviwo Foundation
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
#include <modules/opengl/buffer/bufferobject.h>
#include <inviwo/core/util/rendercontext.h>

#include <vector>

namespace inviwo {

/**
 * @brief Inviwo wrapper for OpengL Vertex Array Objects (VAO)
 *
 * Handles the creation and deletion of OpenGL VAOs. Has functions for attaching and detaching
 * buffer objects. It also keeps track of all attached buffers and corresponding attribute
 * locations.
 * @note An OpenGL Vertex Array Object is tied to a render context and only valid within the same
 * render context that was active when creating it.
 */
class IVW_MODULE_OPENGL_API BufferObjectArray {
public:
    using BindingType = BufferObject::BindingType;

    class IVW_MODULE_OPENGL_API Warn {
        bool value_ = true;
        Warn() = default;

    public:
        explicit Warn(bool enable) : value_{enable} {}
        explicit constexpr operator bool() { return value_; }

        static const Warn Yes;
        static const Warn No;

        friend constexpr bool operator==(Warn a, Warn b) { return b.value_ == a.value_; }
        friend constexpr bool operator!=(Warn a, Warn b) { return b.value_ != a.value_; }
    };

    BufferObjectArray();
    BufferObjectArray(const BufferObjectArray& rhs);
    BufferObjectArray& operator=(const BufferObjectArray& that);
    ~BufferObjectArray();

    /**
     * Return the OpenGL ID of the VAO
     */
    GLuint getId() const;

    /**
     * Bind the VAO
     */
    void bind() const;
    /**
     * Unbind the VAO by binding id 0
     */
    void unbind() const;

    /**
     * @brief Check if this VAO is currently bound
     */
    bool isActive() const;

    /**
     * Removes all buffer attachments from the VAO
     * @pre The BufferObjectArray must be bound
     */
    void clear();

    /**
     * Attach buffer object @p obj to specific location @p location. If @p warn is equal to
     * Warn::Yes, a warning is issued if another buffer object is alread attached to location @p
     * loc.
     * @pre The BufferObjectArray must be bound
     */
    void attachBufferObject(const BufferObject* obj, GLuint location,
                            BindingType bindingType = BindingType::Native, Warn warn = Warn::Yes);

    /**
     * Detach the buffer object at location @p location, if attached, and disable that vertex
     * attribute array.
     * @pre The BufferObjectArray must be bound
     */
    void detachBufferObject(GLuint location);

    BindingType getBindingType(size_t location) const;
    void setBindingType(size_t location, BindingType bindingType);

    const BufferObject* getBufferObject(size_t location = 0) const;

    size_t maxSize() const;

private:
    mutable GLuint id_ = 0;
    std::vector<std::pair<BindingType, const BufferObject*>> attachedBuffers_;

    Canvas::ContextID creationContext_ = nullptr;
};

}  // namespace inviwo
