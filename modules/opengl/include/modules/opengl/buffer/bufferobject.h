/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
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

#include <inviwo/core/datastructures/geometry/geometrytype.h>  // for BufferTarget, BufferTarget...
#include <inviwo/core/util/formats.h>                          // for DataFormatBase
#include <inviwo/core/util/observer.h>                         // for Observable
#include <modules/opengl/buffer/bufferobjectobserver.h>        // for BufferObjectObserver
#include <modules/opengl/glformats.h>                          // for GLFormat, GLFormats
#include <modules/opengl/inviwoopengl.h>                       // for GLenum, GLsizeiptr, GLuint

#include <cstddef>      // for size_t
#include <string_view>  // for string_view
#include <vector>       // for vector

namespace inviwo {

class IVW_MODULE_OPENGL_API BufferObject : public Observable<BufferObjectObserver> {
public:
    /**
     * Type of binding when setting glVertexAttrib*Pointer.
     *
     * See <a
     * href="https://www.khronos.org/opengl/wiki/GLAPI/glVertexAttribPointer">glVertexAttribPointer</a>
     * and <a href="https://www.khronos.org/opengl/wiki/Vertex_Specification#Component_type">Vertex
     * Specification</a> for details.
     */
    enum class BindingType {
        Native,      //!< uses glVertexAttribIPointer for integral types, glVertexAttribDPointer for
                     //!< double, and glVertexAttribPointer otherwise
        ForceFloat,  //!< enforces the use of glVertexAttribPointer independent of the buffer type
        ForceNormalizedFloat  //!< enforces the use of glVertexAttribPointer with normalization of
                              //!< integral types
    };

    /**
     * Policy for managing the buffer size when uploading new data to the GPU.
     *
     * \see upload
     */
    enum class GrowPolicy {
        GrowOnly,    //!< the buffer only grows and will never be resized to a smaller size
        ResizeToFit  //!< the buffer size is adjusted to fit the data exactly
    };

    BufferObject(size_t sizeInBytes, const DataFormatBase* format, BufferUsage usage,
                 BufferTarget target = BufferTarget::Data);

    BufferObject(size_t sizeInBytes, GLFormat format, GLenum usage, GLenum target);

    BufferObject(const BufferObject& rhs);
    BufferObject(BufferObject&& rhs);
    BufferObject& operator=(const BufferObject& other);
    BufferObject& operator=(BufferObject&& other);
    virtual ~BufferObject();
    virtual BufferObject* clone() const;

    GLenum getFormatType() const;
    GLenum getTarget() const;
    GLuint getId() const;

    GLFormat getGLFormat() const;
    const DataFormatBase* getDataFormat() const;

    /**
     * \brief Calls glBindBuffer.
     */
    void bind() const;

    /**
     * \brief Calls glBindBuffer with buffer name 0
     */
    void unbind() const;

    /**
     * \brief Calls glBindBufferBase.
     * Binds the buffer at index of the array of targets
     * specified by the associated target ( @see getTarget )
     * Targets must be one of GL_ATOMIC_COUNTER_BUFFER,
     * GL_TRANSFORM_FEEDBACK_BUFFER, GL_UNIFORM_BUFFER or GL_SHADER_STORAGE_BUFFER
     */
    void bindBase(GLuint index) const;

    /**
     * \brief Calls glBindBufferRange.
     * Binds the range (offset, offset + size) of the buffer at index of the array of targets
     * specified by the associated target ( @see getTarget )
     * Targets must be one of GL_ATOMIC_COUNTER_BUFFER,
     * GL_TRANSFORM_FEEDBACK_BUFFER, GL_UNIFORM_BUFFER or GL_SHADER_STORAGE_BUFFER
     */
    void bindRange(GLuint index, GLintptr offset, GLsizeiptr size) const;

    /**
     * \brief bind the buffer object and set the vertex attribute pointer
     *
     * This will bind the buffer object and then set the respective glVertexAttrib*Pointer.
     * By default, i.e. \p bindingType = BindingType::Native, glVertexAttribIPointer (note the 'I')
     * is used for scalar types and glVertexAttribPointer will be used for floating point
     * types. This behavior can be overwritten by \p bindingType. Then the buffer is only accessible
     * using `float` in the shader.
     *
     * \see BindingType
     *
     * @param location   used to set the vertex attribute location
     * @param bindingType  determines which glVertexAttrib*Pointer is used
     */
    void bindAndSetAttribPointer(GLuint location,
                                 BindingType bindingType = BindingType::Native) const;

    /**
     * Set the size of the buffer in bytes.
     * @param sizeInBytes
     */
    void setSizeInBytes(GLsizeiptr sizeInBytes);
    /**
     * Get the size of the buffer in bytes.
     */
    GLsizeiptr getSizeInBytes() const;

    /**
     * Upload \p data into the buffer. This also binds the buffer. Depending on the grow policy \p
     * policy, the buffer is re-initialized with \p sizeInBytes if it is different from the current
     * size of the buffer.
     * @param data          data to be uploaded. The underlying data must match the current GL
     *                      format of the buffer.
     * @param sizeInBytes   size of the uploaded data
     * @param policy        resizing policy when \p sizeInBytes differs from the current size
     * @see getGLFormat,getSizeInBytes,GrowPolicy
     */
    void upload(const void* data, GLsizeiptr sizeInBytes, GrowPolicy policy = GrowPolicy::GrowOnly);

    /**
     * Upload data from a container \p cont into the buffer. This also binds the buffer.
     * @param data          data to be uploaded. The underlying data must match the current GL
     *                      format of the buffer.
     * @param sizeInBytes   size of the uploaded data
     * @param policy        resizing policy when \p sizeInBytes differs from the current size
     * @see upload(const void*, GLsizeiptr, GrowPolicy)
     */
    template <typename T>
    void upload(const std::vector<T>& cont, GrowPolicy policy = GrowPolicy::GrowOnly) {
        auto sizeInBytes = static_cast<GLsizeiptr>(sizeof(T) * cont.size());
        upload(cont.data(), sizeInBytes, policy);
    }

    void download(void* data) const;

    static std::string_view targetName(GLenum target);

private:
    void initialize(const void* data, GLsizeiptr sizeInBytes);

    GLuint id_;
    GLenum usageGL_;
    GLenum target_;
    GLFormat glFormat_;
    GLsizeiptr sizeInBytes_;
};

inline GLFormat BufferObject::getGLFormat() const { return glFormat_; }
inline const DataFormatBase* BufferObject::getDataFormat() const {
    return DataFormatBase::get(GLFormats::get(glFormat_));
}

}  // namespace inviwo
