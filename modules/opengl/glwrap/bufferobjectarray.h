/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_BUFFER_OBJECT_ARRAY_H
#define IVW_BUFFER_OBJECT_ARRAY_H

#include <modules/opengl/openglmoduledefine.h>
#include <modules/opengl/inviwoopengl.h>

namespace inviwo {

class BufferObject;

class IVW_MODULE_OPENGL_API BufferObjectArray {

public:
    BufferObjectArray();
    BufferObjectArray(const BufferObjectArray& rhs);
    virtual ~BufferObjectArray();
    virtual BufferObjectArray* clone() const;

    void initialize();
    void deinitialize();

    GLuint getId() const;

    void bind() const;
    void unbind() const;

    void clear(); // Make sure the buffer is bound before calling clear.

    /* 
    *  Attach buffer object to generic location based on Buffer::BufferType
    *  or if generic location is occupied, add it to the closest available 
    *  after the range for generic locations.
    */
    int attachBufferObject(const BufferObject*);

    // Attach buffer object to specific location
    void attachBufferObject(const BufferObject*, GLuint);

    const BufferObject* getBufferObject(size_t idx = 0) const;

private:
    void pointToObject(const BufferObject*, GLuint);

private:
    GLuint id_;
    std::vector<const BufferObject*> attachedBuffers_;

    static int maxVertexAttribSize_;
};


} // namespace

#endif // IVW_BUFFER_OBJECT_ARRAY_H
