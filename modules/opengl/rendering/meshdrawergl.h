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

#ifndef IVW_MESHGLDRAWER_H
#define IVW_MESHGLDRAWER_H

#include <modules/opengl/inviwoopengl.h>
#include <modules/opengl/geometry/meshgl.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/rendering/meshdrawer.h>
#include <vector>

namespace inviwo {

class IVW_MODULE_OPENGL_API MeshDrawerGL: public MeshDrawer {

public:
    MeshDrawerGL();
    MeshDrawerGL(const Mesh* mesh);
    MeshDrawerGL(const Mesh* mesh, Mesh::AttributesInfo);
    MeshDrawerGL(const Mesh* mesh, GeometryEnums::DrawType dt, GeometryEnums::ConnectivityType ct);
    virtual ~MeshDrawerGL();

    virtual void draw();
    virtual void draw(GeometryEnums::DrawType dt);

    const MeshGL* getMeshGL() const;

    GLenum getDefaultDrawMode();
    GLenum getDrawMode(GeometryEnums::DrawType, GeometryEnums::ConnectivityType);

    virtual const Mesh* getGeometry() const { return meshToDraw_; }

protected:
    virtual MeshDrawer* create(const Mesh* geom) const {
        return new MeshDrawerGL(geom);
    }
    virtual bool canDraw(const Mesh* geom) const {
        return geom != nullptr;
    }

    virtual void initialize(Mesh::AttributesInfo = Mesh::AttributesInfo());
    void initializeIndexBuffer(const Buffer* indexBuffer, Mesh::AttributesInfo ai);
    void drawArray(GeometryEnums::DrawType) const;
    void drawElements(GeometryEnums::DrawType) const;
    void emptyFunc(GeometryEnums::DrawType dt) const {};

    typedef void (MeshDrawerGL::*DrawFunc)(GeometryEnums::DrawType) const;
    struct DrawMethod {
        DrawFunc drawFunc;
        GLenum drawMode;
        std::vector<const Buffer*> elementBufferList;
    };

    DrawMethod drawMethods_[GeometryEnums::NUMBER_OF_DRAW_TYPES];
    const Mesh* meshToDraw_;
};

} // namespace

#endif // IVW_MESHGLDRAWER_H
