/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <inviwo/core/util/canvas.h>             // for Canvas, Canvas::ContextID
#include <modules/opengl/buffer/bufferobject.h>  // for BufferObject, BufferObject::BindingType
#include <modules/opengl/inviwoopengl.h>         // for GLuint

#include <cstddef>  // for size_t
#include <utility>  // for pair
#include <vector>   // for vector

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
     * Attach buffer object @p obj to a specific location @p location.
     * @pre The BufferObjectArray must be bound
     * @throw Exception       if another buffer object is alread attached to location @p location.
     * @throw RangeException  if @p location is invalid. That is @p location >= maxSize().
     * @see attachBufferObjectForce maxSize
     */
    void attachBufferObject(const BufferObject* obj, GLuint location,
                            BindingType bindingType = BindingType::Native);

    /**
     * Enforce the attachment of buffer object @p obj to a specific location @p location. Overrides
     * any existing binding for this location.
     * @pre The BufferObjectArray must be bound
     * @throw RangeException  if @p location is invalid. That is @p location >= maxSize().
     * @see attachBufferObject maxSize
     */
    void attachBufferObjectEnforce(const BufferObject* obj, GLuint location,
                                   BindingType bindingType = BindingType::Native);

    /**
     * Detach the buffer object at location @p location, if attached, and disable that vertex
     * attribute array.
     * @pre The BufferObjectArray must be bound
     * @throw RangeException  if @p location is invalid. That is @p location >= maxSize().
     */
    void detachBufferObject(GLuint location);

    BindingType getBindingType(size_t location) const;
    void setBindingType(size_t location, BindingType bindingType);

    const BufferObject* getBufferObject(size_t location = 0) const;

    /**
     * Return the maximum number of buffers that can be attached to a VAO as supported by the OpenGL
     * hardware. The result corresponds to \c GL_MAX_VERTEX_ATTRIBS.
     * @return maximum number of buffers that can be attached to this BufferObjectArray
     */
    size_t maxSize() const;

private:
    mutable GLuint id_ = 0;
    std::vector<std::pair<BindingType, const BufferObject*>> attachedBuffers_;

    Canvas::ContextID creationContext_ = nullptr;
};

}  // namespace inviwo
