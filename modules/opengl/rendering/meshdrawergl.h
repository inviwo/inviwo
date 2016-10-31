/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2016 Inviwo Foundation
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
#include <modules/opengl/openglutils.h>

#include <vector>

namespace inviwo {

class IVW_MODULE_OPENGL_API MeshDrawerGL : public MeshDrawer {
public:
    enum class DrawMode {
        NotSpecified = 0,
        Points,
        Lines,
        LineStrip,
        LineLoop,
        LinesAdjacency,
        LineStripAdjacency,
        Triangles,
        TriangleStrip,
        TriangleFan,
        TrianglesAdjacency,
        TriangleStripAdjacency,
        NumberOfDrawModes
    };

    /**
     * \brief This class provides functionality for efficient, subsequent drawing of a mesh. The
     * buffers of the mesh will be bound once this class is instantiated.
     */
    class IVW_MODULE_OPENGL_API DrawObject {
    public:
        DrawObject(const MeshGL* mesh, Mesh::MeshInfo arrayMeshInfo);
        DrawObject(const DrawObject&) = delete;
        DrawObject(DrawObject&&) = default;
        DrawObject& operator=(const DrawObject&) = delete;
        DrawObject& operator=(DrawObject&&) = default;
        ~DrawObject() = default;

        void draw();
        void draw(DrawMode drawMode);

        void draw(std::size_t index);
        void draw(DrawMode drawMode, std::size_t index);


        /** 
         * \brief returns the number of index buffers associated with the mesh representation
         */
        std::size_t size() const;

    private:
        utilgl::Enable<MeshGL> enable_;
        const MeshGL *meshGL_;
        Mesh::MeshInfo arrayMeshInfo_;
    };

    MeshDrawerGL();
    MeshDrawerGL(const Mesh* mesh);
    MeshDrawerGL(const MeshDrawerGL &rhs) = default;
    MeshDrawerGL(MeshDrawerGL&& other) = default;
    virtual ~MeshDrawerGL() = default;

    MeshDrawerGL& operator=(const MeshDrawerGL& other) = default;
    MeshDrawerGL& operator=(MeshDrawerGL&& rhs) = default;

    DrawObject getDrawObject() const;
    static DrawObject getDrawObject(const Mesh *mesh);

    /**
     * \brief draws the mesh using its mesh info. If index buffers are present, the mesh
     * will be rendered with glDrawElements() using those index buffers and the associated draw
     * modes. Otherwise, the entire mesh is rendered using glDrawArrays with the default draw mode
     * returned by Mesh::getDefaultMeshInfo().
     *
     * \see Mesh, Mesh::MeshInfo
     */
    virtual void draw() override;

    /**
    * \brief draws the mesh with the specified draw mode. If index buffers are present, the mesh
    * will be rendered with glDrawElements() using those index buffers. Otherwise, the entire mesh
    * is rendered using glDrawArrays.
    *
    * \see Mesh, DrawMode
    *
    * @param drawMode draw mode used to render the mesh
    */
    virtual void draw(DrawMode drawMode);

    static DrawMode getDrawMode(DrawType, ConnectivityType);
    static GLenum getGLDrawMode(DrawMode);
    static GLenum getGLDrawMode(Mesh::MeshInfo meshInfo);

    virtual const Mesh* getMesh() const override { return meshToDraw_; }

protected:
    virtual MeshDrawer* create(const Mesh* geom) const override { return new MeshDrawerGL(geom); }
    virtual bool canDraw(const Mesh* geom) const override { return geom != nullptr; }

    const Mesh* meshToDraw_;
};

}  // namespace

#endif  // IVW_MESHGLDRAWER_H
