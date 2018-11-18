/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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
#include <modules/opengl/buffer/bufferobject.h>

#include <vector>

namespace inviwo {

class IVW_MODULE_OPENGL_API BufferObjectArray {
public:
    using BindingType = BufferObject::BindingType;

    BufferObjectArray();
    BufferObjectArray(const BufferObjectArray& rhs);
    BufferObjectArray& operator=(const BufferObjectArray& that);
    ~BufferObjectArray();

    GLuint getId() const;

    void bind() const;
    void unbind() const;

    void clear();  // Make sure the buffer is bound before calling clear.

    // Attach buffer object to specific location
    void attachBufferObject(const BufferObject*, GLuint,
                            BindingType bindingType = BindingType::Native);

    BindingType getBindingType(size_t location) const;
    void setBindingType(size_t location, BindingType bindingType);

    const BufferObject* getBufferObject(size_t location = 0) const;

    size_t maxSize() const;

private:
    mutable bool reattach_ = true;
    mutable GLuint id_ = 0;
    std::vector<std::pair<BindingType, const BufferObject*>> attachedBuffers_;
};

}  // namespace inviwo

#endif  // IVW_BUFFER_OBJECT_ARRAY_H
