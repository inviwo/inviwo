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

#ifndef IVW_MESHGL_H
#define IVW_MESHGL_H

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <modules/opengl/geometry/geometrygl.h>

namespace inviwo {

class BufferGL;
class BufferObjectArray;

class IVW_MODULE_OPENGL_API MeshGL : public GeometryGL {

public:
    MeshGL();
    MeshGL(const MeshGL& rhs);
    virtual ~MeshGL();
    virtual void initialize();
    virtual void deinitialize();
    virtual MeshGL* clone() const;

    void enable() const;
    void disable() const;

    const BufferGL* getBufferGL(size_t idx=0) const;

    virtual Mesh* getOwner();
    virtual const Mesh* getOwner() const;

protected:
    virtual void update(bool editable);

private:
    std::vector<const BufferGL*> attributesGL_;
    BufferObjectArray* attributesArray_;

};

} // namespace

#endif // IVW_MESHGL_H
