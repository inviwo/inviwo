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

class IVW_MODULE_OPENGL_API MeshDrawerGL : public MeshDrawer {
public:
    enum class DrawMode {
        NOT_SPECIFIED = 0,
        POINTS,
        LINES,
        LINE_STRIP,
        LINE_LOOP,
        LINES_ADJACENCY,
        LINE_STRIP_ADJACENCY,
        TRIANGLES,
        TRIANGLE_STRIP,
        TRIANGLE_FAN,
        TRIANGLES_ADJACENCY,
        TRIANGLE_STRIP_ADJACENCY,
        NUMBER_OF_DRAW_MODES
    };

    MeshDrawerGL();
    MeshDrawerGL(const Mesh* mesh);
    MeshDrawerGL(const Mesh* mesh, Mesh::MeshInfo);
    MeshDrawerGL(const Mesh* mesh, DrawType dt, ConnectivityType ct);
    MeshDrawerGL& operator=(const MeshDrawerGL& other);
    MeshDrawerGL(MeshDrawerGL&& other);  // move constructor
    virtual ~MeshDrawerGL();

    virtual void draw() override;
    virtual void draw(DrawMode dm);

    GLenum getDefaultDrawMode();
    DrawMode getDrawMode(DrawType, ConnectivityType) const;
    GLenum getGLDrawMode(DrawMode) const;

    virtual const Mesh* getGeometry() const override { return meshToDraw_; }

protected:
    virtual MeshDrawer* create(const Mesh* geom) const override { return new MeshDrawerGL(geom); }
    virtual bool canDraw(const Mesh* geom) const override { return geom != nullptr; }

    virtual void initialize(Mesh::MeshInfo = Mesh::MeshInfo());
    void initializeIndexBuffer(const BufferBase* indexBuffer, Mesh::MeshInfo ai);

    void drawArray(DrawMode) const;
    void drawElements(DrawMode) const;
    void emptyFunc(DrawMode dt) const {};

    // A member function pointer to Either drawArrays, drawElement or emptyFunc
    using DrawFunc = void (MeshDrawerGL::*)(DrawMode) const;
    struct DrawMethod {
        DrawFunc drawFunc;
        GLenum drawMode;
        std::vector<const BufferBase*> elementBufferList;
    };

    DrawMethod drawMethods_[static_cast<size_t>(DrawMode::NUMBER_OF_DRAW_MODES)];
    const Mesh* meshToDraw_;
};

}  // namespace

#endif  // IVW_MESHGLDRAWER_H
