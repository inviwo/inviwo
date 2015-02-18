/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_BUFFERGL_H
#define IVW_BUFFERGL_H

#include <modules/opengl/openglmoduledefine.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/buffer/bufferglobjectid.h>
#include <modules/opengl/glwrap/bufferobject.h>
#include <modules/opengl/glwrap/bufferobjectarray.h>
#include <inviwo/core/datastructures/buffer/bufferrepresentation.h>

namespace inviwo {

class IVW_MODULE_OPENGL_API BufferGL: public BufferRepresentation {

public:
    BufferGL(size_t size, const DataFormatBase* format, BufferType type, BufferUsage usage, BufferObject* data = NULL);
    BufferGL(const BufferGL& rhs);
    virtual ~BufferGL();
    virtual BufferGL* clone() const;
    
    virtual void initialize();
    virtual void deinitialize();

    virtual void setSize(size_t size);

    GLenum getFormatType() const;
    GLuint getId() const;

    BufferObject* getBufferObject() { return buffer_; }
    const BufferObject* getBufferObject() const { return buffer_; }

    void bind() const;

    void initialize(const void* data, GLsizeiptr sizeInBytes);
    void upload(const void* data, GLsizeiptr sizeInBytes);

    void download(void* data) const;

    void enable() const;
    void disable() const;

private:
    BufferObject* buffer_;
    mutable BufferObjectArray* bufferArray_;
};


} // namespace

#endif // IVW_BUFFERGL_H
