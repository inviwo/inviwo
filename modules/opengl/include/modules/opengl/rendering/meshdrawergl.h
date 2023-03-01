/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2023 Inviwo Foundation
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

#include <inviwo/core/datastructures/geometry/geometrytype.h>  // for ConnectivityType, DrawType
#include <inviwo/core/datastructures/geometry/mesh.h>          // for Mesh, Mesh::MeshInfo
#include <inviwo/core/rendering/meshdrawer.h>                  // for MeshDrawer
#include <modules/opengl/geometry/meshgl.h>                    // for MeshGL
#include <modules/opengl/inviwoopengl.h>                       // for GLenum
#include <modules/opengl/openglutils.h>                        // for Enable

#include <cstddef>  // for size_t

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
        DrawObject(const Mesh& mesh);
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

        void drawInstanced(size_t instances);
        void drawInstanced(DrawMode drawMode, size_t instances);
        void drawInstanced(std::size_t index, size_t instances);
        void drawInstanced(DrawMode drawMode, std::size_t index, size_t instances);

        void drawOnly(DrawMode drawMode);
        void drawOnlyInstanced(DrawMode drawMode, size_t instances);
        void drawOnly(std::function<bool(const Mesh::MeshInfo&)> filter);
        void drawOnlyInstanced(std::function<bool(const Mesh::MeshInfo&)> filter, size_t instances);

        /**
         * \brief returns the number of index buffers associated with the mesh representation
         */
        std::size_t size() const;

    private:
        void checkIndex(size_t index) const;

        utilgl::Enable<MeshGL> enable_;
        const MeshGL* meshGL_;
        Mesh::MeshInfo arrayMeshInfo_;
    };

    MeshDrawerGL();
    MeshDrawerGL(const Mesh* mesh);
    MeshDrawerGL(const MeshDrawerGL& rhs) = default;
    MeshDrawerGL(MeshDrawerGL&& other) = default;
    virtual ~MeshDrawerGL() = default;

    MeshDrawerGL& operator=(const MeshDrawerGL& other) = default;
    MeshDrawerGL& operator=(MeshDrawerGL&& rhs) = default;

    DrawObject getDrawObject() const;
    static DrawObject getDrawObject(const Mesh* mesh);

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
    static DrawMode getDrawMode(Mesh::MeshInfo meshInfo);
    static GLenum getGLDrawMode(DrawMode);
    static GLenum getGLDrawMode(Mesh::MeshInfo meshInfo);

    virtual const Mesh* getMesh() const override { return meshToDraw_; }

protected:
    virtual MeshDrawer* create(const Mesh* geom) const override { return new MeshDrawerGL(geom); }
    virtual bool canDraw(const Mesh* geom) const override { return geom != nullptr; }

    const Mesh* meshToDraw_;
};

}  // namespace inviwo
