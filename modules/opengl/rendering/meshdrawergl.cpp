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

#include <modules/opengl/buffer/elementbuffergl.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/openglutils.h>

#include <inviwo/core/util/exception.h>

namespace inviwo {

MeshDrawerGL::MeshDrawerGL() : meshToDraw_(nullptr) {}

MeshDrawerGL::MeshDrawerGL(const Mesh* mesh)
    : meshToDraw_(mesh), meshInfo_(DrawType::NotSpecified, ConnectivityType::None) {
    if (mesh == nullptr) throw NullPointerException("input mesh is null", IvwContext);
    meshInfo_ = mesh->getDefaultMeshInfo();
}

MeshDrawerGL::MeshDrawerGL(const Mesh* mesh, Mesh::MeshInfo ai) : meshToDraw_(mesh), meshInfo_(ai) {
    if (mesh == nullptr) throw NullPointerException("input mesh is null", IvwContext);
}

MeshDrawerGL::MeshDrawerGL(const Mesh* mesh, DrawType dt, ConnectivityType ct)
    : meshToDraw_(mesh)
    , meshInfo_(Mesh::MeshInfo(dt, ct)) {
    if (mesh == nullptr) throw NullPointerException("input mesh is null", IvwContext);
}

MeshDrawerGL::MeshDrawerGL(MeshDrawerGL&& other)
    : MeshDrawer(std::move(other)), meshToDraw_{other.meshToDraw_}, meshInfo_{other.meshInfo_} {
    other.meshToDraw_ = nullptr;
}

MeshDrawerGL::~MeshDrawerGL() = default;

MeshDrawerGL& MeshDrawerGL::operator=(const MeshDrawerGL& rhs) {
    if (this != &rhs) {
        MeshDrawer::operator=(rhs);
        meshToDraw_ = rhs.meshToDraw_;
        meshInfo_ = rhs.meshInfo_;
    }
    return *this;
}

MeshDrawerGL& MeshDrawerGL::operator=(MeshDrawerGL&& rhs) {
    if (this != &rhs) {
        MeshDrawer::operator=(rhs);
        meshToDraw_ = std::move(rhs.meshToDraw_);
        meshInfo_ = rhs.meshInfo_;
    }
    return *this;
}

void MeshDrawerGL::draw() {
    auto meshGL = meshToDraw_->getRepresentation<MeshGL>();
    utilgl::Enable<MeshGL> enable(meshGL);

    std::size_t numIndexBuffers = meshGL->getIndexBufferCount();
    if (numIndexBuffers > 0) {
        // draw mesh using the index buffers
        for (std::size_t i = 0; i < numIndexBuffers; ++i) {
            auto indexBuffer = meshGL->getIndexBuffer(i);
            auto numIndices = indexBuffer->getSize();
            if (numIndices > 0) {
                indexBuffer->bind();
                auto drawMode = getGLDrawMode(meshGL->getMeshInfoForIndexBuffer(i));
                glDrawElements(drawMode, static_cast<GLsizei>(numIndices),
                               indexBuffer->getFormatType(), nullptr);
            }
        }
    } else {
        // the mesh does not contain index buffers, render all vertices
        auto drawMode = getGLDrawMode(meshToDraw_->getDefaultMeshInfo());
        glDrawArrays(drawMode, 0, static_cast<GLsizei>(meshGL->getBufferGL(0)->getSize()));
    }
}

void MeshDrawerGL::draw(DrawMode drawMode) {
    auto meshGL = meshToDraw_->getRepresentation<MeshGL>();
    utilgl::Enable<MeshGL> enable(meshGL);

    auto drawModeGL = getGLDrawMode(meshToDraw_->getDefaultMeshInfo());

    std::size_t numIndexBuffers = meshGL->getIndexBufferCount();
    if (numIndexBuffers > 0) {
        // draw mesh using the index buffers
        for (std::size_t i = 0; i < numIndexBuffers; ++i) {
            auto indexBuffer = meshGL->getIndexBuffer(i);
            auto numIndices = indexBuffer->getSize();
            if (numIndices > 0) {
                indexBuffer->bind();
                glDrawElements(drawModeGL, static_cast<GLsizei>(numIndices),
                               indexBuffer->getFormatType(), nullptr);
            }
        }
    }
    else {
        // the mesh does not contain index buffers, render all vertices
        glDrawArrays(drawModeGL, 0, static_cast<GLsizei>(meshGL->getBufferGL(0)->getSize()));
    }
}

GLenum MeshDrawerGL::getDefaultDrawMode() { return getGLDrawMode(meshInfo_); }

MeshDrawerGL::DrawMode MeshDrawerGL::getDrawMode(DrawType dt, ConnectivityType ct) const {
    switch (dt) {
        case DrawType::Triangles:
            switch (ct) {
                case ConnectivityType::None:
                    return DrawMode::Triangles;

                case ConnectivityType::Strip:
                    return DrawMode::TriangleStrip;

                case ConnectivityType::Fan:
                    return DrawMode::TriangleFan;

                case ConnectivityType::Adjacency:
                    return DrawMode::TrianglesAdjacency;

                case ConnectivityType::StripAdjacency:
                    return DrawMode::TriangleStripAdjacency;

                case ConnectivityType::Loop:
                case ConnectivityType::NumberOfConnectivityTypes:
                default:
                    return DrawMode::Points;
            }

        case DrawType::Lines:
            switch (ct) {
                case ConnectivityType::None:
                    return DrawMode::Lines;

                case ConnectivityType::Strip:
                    return DrawMode::LineStrip;

                case ConnectivityType::Loop:
                    return DrawMode::LineLoop;

                case ConnectivityType::Adjacency:
                    return DrawMode::LinesAdjacency;

                case ConnectivityType::StripAdjacency:
                    return DrawMode::LineStripAdjacency;

                case ConnectivityType::Fan:
                case ConnectivityType::NumberOfConnectivityTypes:
                default:
                    return DrawMode::Points;
            }

        case DrawType::Points:
        case DrawType::NotSpecified:
        case DrawType::NumberOfDrawTypes:
        default:
            return DrawMode::Points;
    }
}

GLenum MeshDrawerGL::getGLDrawMode(DrawMode dm) const {
    switch (dm) {
        case DrawMode::Points:
            return GL_POINTS;
        case DrawMode::Lines:
            return GL_LINES;
        case DrawMode::LineStrip:
            return GL_LINE_STRIP;
        case DrawMode::LineLoop:
            return GL_LINE_LOOP;
        case DrawMode::LinesAdjacency:
            return GL_LINES_ADJACENCY;
        case DrawMode::LineStripAdjacency:
            return GL_LINE_STRIP_ADJACENCY;
        case DrawMode::Triangles:
            return GL_TRIANGLES;
        case DrawMode::TriangleStrip:
            return GL_TRIANGLE_STRIP;
        case DrawMode::TriangleFan:
            return GL_TRIANGLE_FAN;
        case DrawMode::TrianglesAdjacency:
            return GL_TRIANGLES_ADJACENCY;
        case DrawMode::TriangleStripAdjacency:
            return GL_TRIANGLE_STRIP_ADJACENCY;
        case DrawMode::NumberOfDrawModes:
        case DrawMode::NotSpecified:
        default:
            return GL_POINTS;
    }
}

GLenum MeshDrawerGL::getGLDrawMode(Mesh::MeshInfo meshInfo) const {
    return getGLDrawMode(getDrawMode(meshInfo.dt, meshInfo.ct));
}

}  // namespace
