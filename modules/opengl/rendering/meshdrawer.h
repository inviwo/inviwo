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

#include <modules/opengl/geometry/meshgl.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/rendering/geometrydrawer.h>
#include <vector>

namespace inviwo {

class IVW_MODULE_OPENGL_API MeshDrawer: public GeometryDrawer {

public:
    MeshDrawer();
    MeshDrawer(const Mesh* mesh);
    MeshDrawer(const Mesh* mesh, Mesh::AttributesInfo);
    MeshDrawer(const Mesh* mesh, GeometryEnums::DrawType dt, GeometryEnums::ConnectivityType ct);
    virtual ~MeshDrawer();

    virtual void draw();
    virtual void draw(GeometryEnums::DrawType dt);

    const MeshGL* getMeshGL() const;

    GLenum getDefaultDrawMode();
    GLenum getDrawMode(GeometryEnums::DrawType, GeometryEnums::ConnectivityType);

    virtual const Geometry* getGeometry() const { return meshToDraw_; }

protected:
    virtual GeometryDrawer* create(const Geometry* geom) const {
        return new MeshDrawer(static_cast<const Mesh*>(geom));
    }
    virtual bool canDraw(const Geometry* geom) const {
        return dynamic_cast<const Mesh*>(geom) != NULL;
    }

    virtual void initialize(Mesh::AttributesInfo = Mesh::AttributesInfo());
    void initializeIndexBuffer(const Buffer* indexBuffer, Mesh::AttributesInfo ai);
    void drawArray(GeometryEnums::DrawType) const;
    void drawElements(GeometryEnums::DrawType) const;
    void emptyFunc(GeometryEnums::DrawType dt) const {};

    typedef void (MeshDrawer::*DrawFunc)(GeometryEnums::DrawType) const;
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
